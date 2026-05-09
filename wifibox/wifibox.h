#pragma once

#include <map>

#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"

#include "comm.h"
#include "event.h"
#include "sensor.h"

namespace esphome
{
    namespace wifibox
    {
        class Wifibox : public PollingComponent, public uart::UARTDevice
        {
        public:
            Wifibox(uart::UARTComponent *parent) : PollingComponent(), uart::UARTDevice(parent)
            {
                ering.init();
                comm = new Wifibox_Comm(&ering);
            }

            void update() override;

            void publish_sensor();
            void register_sensor(IWifiBoxSensor *sensor);

        protected:
            float get_setup_priority() const override { return esphome::setup_priority::LATE; }
            std::map<std::string, IWifiBoxSensor *> sensors;

        private:
            Wifibox_Comm *comm;
            EventRing ering;
        };
    }
}
