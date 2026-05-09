#pragma once

#include <string>

#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome
{
    namespace wifibox
    {
        class IWifiBoxSensor
        {
        public:
            virtual const std::string &get_key() const = 0;
            virtual void publish_val(const char *value) = 0;
        };

        class WifiboxSensorBase : public IWifiBoxSensor
        {
        public:
            const std::string &get_key() const override
            {
                return m_key;
            }

        protected:
            std::string m_key;
        };

        class WifiboxTextSensor : public WifiboxSensorBase, public text_sensor::TextSensor, public Component
        {
        public:
            WifiboxTextSensor(std::string key)
            {
                m_key = key;
            }

            void publish_val(const char *value) override
            {
                publish_state(value);
            }
        };

        class WifiboxSensor : public WifiboxSensorBase, public sensor::Sensor, public Component
        {
        public:
            WifiboxSensor(std::string key)
            {
                m_key = key;
            }

            void publish_val(const char *value)
            {
                publish_state(simpleatof(value));
            }

            double simpleatof(const char *value)
            {
                int idx = 0;
                double result = 0.0;
                int negative = 0;

                if (value[idx] == '-')
                {
                    negative = 1;
                    idx++;
                }

                while (value[idx] != '.' && value[idx] != '\0')
                {
                    if (value[idx] >= '0' && value[idx] <= '9')
                    {
                        result = result * 10.0 + (value[idx] - '0');
                    }
                    idx++;
                }

                if (value[idx] == '.')
                {
                    idx++;
                    double divisor = 10.0;
                    while (value[idx] != '\0')
                    {
                        if (value[idx] >= '0' && value[idx] <= '9')
                        {
                            result += (value[idx] - '0') / divisor;
                            divisor *= 10.0;
                        }
                        idx++;
                    }
                }

                return negative ? -result : result;
            }
        };
    }
}
