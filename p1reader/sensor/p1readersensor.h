#pragma once

#include <string>

#include "esphome/components/sensor/sensor.h"

#include "../p1reader.h"

namespace esphome
{
    namespace p1reader
    {
        class P1ReaderSensor : public P1ReaderSensorBase, public sensor::Sensor, public Component
        {
        public:
            P1ReaderSensor(std::string obis_code) {
                _obis_code = obis_code;
            }

            void publish_val(const char* value) override {
                publish_state(simpleatof(value));
            }

            double simpleatof(const char* value) {
                int idx = 0;
                double result = 0.0;
                int negative = 0;

                if (value[idx] == '-') {
                    negative = 1;
                    idx++;
                }

                while (value[idx] != '.' && value[idx] != '\0') {
                    if (value[idx] >= '0' && value[idx] <= '9') {
                        result = result * 10.0 + (value[idx] - '0');
                    }
                    idx++;
                }

                if (value[idx] == '.') {
                    idx++;
                    double divisor = 10.0;
                    while (value[idx] != '\0') {
                        if (value[idx] >= '0' && value[idx] <= '9') {
                            result += (value[idx] - '0') / divisor;
                            divisor *= 10.0;
                        }
                        idx++;
                    }
                }

                return negative ? -result : result;
            }
        };
    }  // namespace p1reader
}  // namespace esphome
