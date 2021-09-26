#ifndef RF24_NANLINK_H_
#define RF24_NANLINK_H_

#include <cstring>
#include "esphome.h"
#include "esphome/components/spi/spi.h"
#include "RF24.h"

#define SCK_PIN 22
#define MISO_PIN 19
#define MOSI_PIN 23
#define CS_PIN 33
#define CE_PIN 21 // or we can just wire this to high?

class RF24NanlinkSpiComponent : public Component, public CustomMQTTDevice {
  private:
    RF24 radio;

    uint8_t seqno = 0;

    uint8_t default_channel = 4;

  public:
    void setup() override {
      SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);
      radio.begin(&SPI, CE_PIN, CS_PIN);
      radio.setAutoAck(0, true);
      radio.setChannel(0x3c);
      radio.setRetries(1, 15);
      radio.setPALevel(3, false);
      subscribe("nanlink/ch4", &RF24NanlinkSpiComponent::on_message);
    }

    void on_message(const std::string &payload) {
      const char *cmd = payload.c_str();
      uint8_t channel = default_channel;
      uint8_t args[5] = {0};
      uint8_t times = 1;
      if (sscanf(cmd, "raw_cmd %hhu %hhu %hhu %hhu %hhu", args, args+1, args+2,
                 args+3, args+4) > 3) {

        ESP_LOGD("rf24nanlink",
                 "raw_cmd called with: 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x",
                 args[0], args[1], args[2], args[3], args[4]);

        if (args[4])
          channel = args[4];

        send_payload(channel, args[0], args[1], args[2], args[3]);

      } else if (sscanf(cmd, "power %hhu", &channel) == 1) {

        ESP_LOGD("rf24nanlink", "power called on channel %d", channel);
        send_payload(channel, 1, 1, 1, 0, 0);

      } else if (sscanf(cmd, "dim_up %hhu %hhu", args, args+1) > 0) {

        if (args[0])
          times = args[0];

        if (args[1])
          channel = args[1];

        ESP_LOGD("rf24nanlink", "calling dim_up %d times on channel %d", times, channel);

        send_payload(channel, 3, 1, 0, 0, times);

      } else if (sscanf(cmd, "dim_down %hhu %hhu", args, args+1) > 0) {

        if (args[0])
          times = args[0];

        if (args[1])
          channel = args[1];

        ESP_LOGD("rf24nanlink", "calling dim_down %d times on channel %d", times, channel);

        // Same comment for byte11 as for dim_up
        send_payload(channel, 3, 2, 0, 0, times);

      } else if (sscanf(cmd, "cct_mode %hhu", args) > 0) {
        if (args[0])
          channel = args[0];

        send_payload(channel, 2, 1, 1, 0, 1);

      } else {
        ESP_LOGD("rf24nanlink", "unknown input: %s", cmd);
      }
    }

  private:
    // Don't think b13 is actually anything, needs more experimentation
    void send_payload(uint8_t channel, uint8_t b7, uint8_t b9, uint8_t b11,
                      uint8_t b13, int repeats = 1) {
      ESP_LOGD("rf24nanlink", "Sending %d rf24 payload(s), seq number: %d",
               repeats, seqno);
      uint8_t tx_channel[5] = { 0, 0, 0, 0 , channel};
      uint8_t message[32] = {0x0, 0x1, 0x1, 0xf, seqno};
      uint8_t checksum = 1;

      message[7] = b7;
      message[9] = b9;
      message[11] = b11;
      message[13] = b13;

      for (int i = 0; i < 14; ++i) {
        checksum += message[i];
      }
      message[14] = checksum;

      radio.openReadingPipe(0, tx_channel);
      radio.openWritingPipe(tx_channel);
      for (int i = 0; i < repeats; ++i) {
        radio.write(message, 32);
        // Large consecutive writes don't seem to be bueno
        if (i > 0) {
          delay(5);
        }
        seqno++;
      }
      if (repeats > 1) {
        ESP_LOGD("rf24nanlink", "Next seq number will be: %d", seqno);
      }
    }
};

#endif // RF24_NANLINK_H_
