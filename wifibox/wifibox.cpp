#include "esphome/core/log.h"
#include "wifibox.h"
#include <stdio.h>

namespace esphome
{
    namespace wifibox
    {
        void Wifibox::update()
        {
            comm->communication();

            uint8_t c;

            while (comm->available())
            {
                c = comm->read();
                write_byte(c);
            }

            while (available())
            {
                read_byte(&c);
                comm->write(c);
            }

            publish_sensor();
        }

        void Wifibox::register_sensor(IWifiBoxSensor *sensor)
        {
            sensors.emplace(sensor->get_key(), sensor);
        }

        void Wifibox::publish_sensor()
        {
            Event e;
            while (ering.read(e))
            {
                ESP_LOGI("sensor", "event for %s with %s", e.key, e.value);

                auto it = sensors.find(e.key);
                if (it != sensors.end())
                    it->second->publish_val(e.value);

                if (e.key[0] == 'E' || e.key[0] == 'W')
                {
                    auto it = sensors.find("status");
                    if (it != sensors.end())
                    {
                        if (e.value[0] == 'P')
                        {
                            it->second->publish_val("");
                        }
                        else
                        {
                            it->second->publish_val(e.key);
                        }
                    }
                }
            }
        }
    }
}
