#pragma once

#include "Adafruit_NeoKey_1x4.h"
#include "seesaw_neopixel.h"
#include "esphome.h"

class NeoKey1x4Component : public PollingComponent {
        NeoKey1x4Component(uint32_t color,
                           BinarySensor *button1,
                           BinarySensor *button2,
                           BinarySensor *button3,
                           BinarySensor *button4) :
            PollingComponent(40),
            color(color) {

            sensors.insert(sensors.end(), { button1, button2, button3, button4 });

        }

    public:
        void setup() override {

            if (!neokey.begin(0x30)) {
                ESP_LOGE("neokeycomp", "Could not start NeoKey");
                return;
            }
            ready = true;
        }

        void update() override {
            if (!ready)
                return;

            uint8_t buttons = neokey.read();

            for (int i = 0; i < 4; ++i) {

              if (buttons & (1 << i)) {
                sensors[i]->publish_state(true);
                neokey.pixels.setPixelColor(i, color);
              } else {
                sensors[i]->publish_state(false);
                neokey.pixels.setPixelColor(i, 0);
              }
            }
            neokey.pixels.show();
        }

        Adafruit_NeoKey_1x4 neokey;
        bool ready = false;
        uint32_t color;
        std::vector<BinarySensor*> sensors;
};
