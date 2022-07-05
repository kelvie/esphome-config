// -*-c++-*-

#ifndef ACURITE_00986_H_
#define ACURITE_00986_H_

#include "esphome.h"

#include <cstdio>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

class Acurite00986Component : public CustomMQTTDevice, public Component {
public:
  // Will add <sensor_num>-<sensor_id> to the topic prefix
  Acurite00986Component(gpio_num_t pin, const std::string &topic_prefix)
      : topic_prefix_(topic_prefix) {
    auto idfpin = new esp32::IDFInternalGPIOPin;
    idfpin->set_pin(pin);
    // 20mA
    idfpin->set_drive_strength(GPIO_DRIVE_CAP_2);
    idfpin->set_inverted(false);
    idfpin->set_flags(gpio::FLAG_INPUT | gpio::FLAG_PULLUP);
    pin_ = idfpin;

    queue_ = xQueueCreate(16, sizeof(char[5]));
    log_queue_ = xQueueCreate(20, sizeof(int));
  }

  void setup() override {
    pin_->setup();
    isr_pin_ = pin_->to_isr();
    bit_index_ = 0;
    msg_index_ = 0;
    sync_cycles_ = 0;
    edge_after_sync_ = true;
    pin_->attach_interrupt(Acurite00986Component::gpio_isr, this,
                           gpio::INTERRUPT_ANY_EDGE);
    last_edge_ = micros();
  }

  int parse_temp_celsius(char n) {
    int magnitude_f = n & (~0x80);
    magnitude_f *= n & 0x80 ? -1 : 1;

    return (magnitude_f - 32) * 100 / 180;
  }

  void loop() override {
    char msg[5];
    while (xQueueReceive(queue_, msg, 0) == pdTRUE) {
      ESP_LOGD(TAG, "Sensor %d (id: %.2x%.2x) %d°C.%s", msg[3] & 1 ? 2 : 1,
               msg[1], msg[2], parse_temp_celsius(msg[0]),
               msg[3] & 2 ? " Low battery." : "");

      // Topic name:
      // <topic_prefix_>-<sensor_num>-<sensor_id>
      char topic_suffix[] = "-1-abcd";
      snprintf(topic_suffix, sizeof(topic_suffix), "%d-%.2x%.2x",
               msg[3] & 1 ? 2 : 1, msg[1], msg[2]);
      this->publish_json(topic_prefix_ + topic_suffix, [=](JsonObject root) {
        root["temp_c"] = parse_temp_celsius(msg[0]);
        root["low_battery"] = msg[3] & 2 ? 1 : 0;
      });
    }

    int log_sync_cycle;
    while (xQueueReceive(log_queue_, &log_sync_cycle, 0) == pdTRUE) {
      ESP_LOGD(TAG, "debug value: %d", log_sync_cycle);
    }
  }

  // constant
  static const uint32_t sync_width_us_ = 1600;
  static const uint32_t logic_high_width_us_ = 1100;
  static const uint32_t logic_low_width_us_ = 750;

  // the width can be +/- this amount in micros
  static const uint32_t width_tolerance_us_ = 200;

  // For logging
  static constexpr const char *TAG = "acurite00986";

protected:
  static bool inline compare_pulse(uint32_t a, uint32_t b) {
    if (a > b) {
      return (a - b) < width_tolerance_us_;
    }
    return (b - a) < width_tolerance_us_;
  }

  // Interrupt handler, so let's keep it short and only access volatiles.
  static void IRAM_ATTR gpio_isr(Acurite00986Component *c) {
    const uint32_t now = micros();
    const bool rising = c->isr_pin_.digital_read();
    volatile char *cur = c->msg_ + c->msg_index_;

    // For sending values to the main loop to log for debugging
    int debug_send;

    // TODO: decide on skip if necessary to save cycles

    // We are not synced util we have seen 7 edges spaced sync_width_us apart
    if (c->sync_cycles_ < 7) {
      if (c->compare_pulse(now - c->last_edge_, sync_width_us_)) {
        c->sync_cycles_++;
        goto end;
      }

      c->sync_cycles_ = 0;
      goto end;
    }

    // TODO: check CRC and the second copy of the message
    // https://github.com/merbanan/rtl_433/blob/master/src/devices/acurite.c

    if (!rising) {
      // don't set last_edge, as we only care about rising edges for this
      // timing.
      return;
    }

    // On the first rising edge (after syncing), just record the edge.
    if (c->edge_after_sync_) {
      c->edge_after_sync_ = false;
      goto end;
    }

    // For debugging, to send various values from the ISR
    // debug_send = now;
    // xQueueSendFromISR(c->log_queue_, &debug_send, NULL);

    // At this point, we're measuring rising to rising for logic highs and
    // lows. Bytes are sent LSB first
    if (c->compare_pulse(now - c->last_edge_, logic_high_width_us_)) {
      *cur = (*cur >> 1) + 0x80;
      c->bit_index_++;
    } else if (c->compare_pulse(now - c->last_edge_, logic_low_width_us_)) {
      *cur = *cur >> 1;
      c->bit_index_++;
    } else {
      // Not high, not low, probably noise.
      c->msg_index_ = 0;
      c->bit_index_ = 0;
      c->sync_cycles_ = 0;
      c->edge_after_sync_ = true;
      goto end;
    }

    // Move onto next byte if 8 bits have been written
    if (c->bit_index_ > 7) {
      c->msg_index_++;
      c->bit_index_ = 0;
    }

    // Send the full message after all 5 messages are received
    if (c->msg_index_ >= 5) {
      char msg[5];

      // memcpy(msg, c->msg_, 5);
      for (int i = 0; i < sizeof(msg); i++) {
        msg[i] = c->msg_[i];
      }

      xQueueSendFromISR(c->queue_, msg, NULL);
      c->msg_index_ = 0;
      c->bit_index_ = 0;
      c->sync_cycles_ = 0;
      c->edge_after_sync_ = true;
    }

  end:
    c->last_edge_ = now;
    return;
  }

private:
  const std::string topic_prefix_;

  InternalGPIOPin *pin_ = nullptr;
  ISRInternalGPIOPin isr_pin_;

  // Time (us) of the last edge
  volatile uint32_t last_edge_;

  // How many sync cycles we're in (7 edges within the time means it's synced)
  volatile int sync_cycles_;
  // The message, optionally repeated twice
  volatile char msg_[10];
  volatile int msg_index_;
  volatile int bit_index_;
  volatile bool edge_after_sync_;
  volatile QueueHandle_t queue_;

  // For debugging the ISR
  volatile QueueHandle_t log_queue_;
};

#endif // ACURITE_00986_H
