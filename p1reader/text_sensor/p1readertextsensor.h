#pragma once

#include <string>

#include "esphome/components/text_sensor/text_sensor.h"

#include "../p1reader.h"

namespace esphome
{
    namespace p1reader
    {
        class P1ReaderTextSensor : public P1ReaderSensorBase, public text_sensor::TextSensor, public Component
        {
        public:
            P1ReaderTextSensor(std::string obis_code) {
                _obis_code = obis_code;
            }

            void publish_val(const char* value) override {
                publish_state(value);
            }
        };
    }  // namespace p1reader
}  // namespace esphome
