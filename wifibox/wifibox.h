#pragma once

#include <map>

#include "esphome/components/sensor/sensor.h"

#include "comm.h"
#include "event.h"
#include "sensor.h"

namespace esphome
{
    namespace wifibox
    {
        class Wifibox : public Component
        {
        public:
            Wifibox_Comm *comm;

            void setup() override;
            void loop() override;

            void publish_sensor();
            void register_sensor(IWifiBoxSensor *sensor);
            void push_json(const char *json);

            static void rs485_worker_task(void *parameters);

        protected:
            std::map<std::string, IWifiBoxSensor *> sensors;

        private:
            EventRing ering;
        };
    }
}
