#include "esphome/core/log.h"
#include "wifibox.h"
#include <stdio.h>

namespace esphome
{
    namespace wifibox
    {
        void Wifibox::setup()
        {
            uart_config_t uart_config = {
                .baud_rate = 57600,
                .data_bits = UART_DATA_8_BITS,
                .parity = UART_PARITY_DISABLE,
                .stop_bits = UART_STOP_BITS_1,
                .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
            uart_param_config(UART_NUM_2, &uart_config);
            // RX=16, TX=17, RTS=18 (if DE/RE)
            uart_set_pin(UART_NUM_2, 17, 16, 18, UART_PIN_NO_CHANGE);
            // Hardware buffer size
            uart_driver_install(UART_NUM_2, BUF_SIZE, BUF_SIZE, 0, NULL, 0);
            // Enable RS485 half-duplex mode
            uart_set_mode(UART_NUM_2, UART_MODE_RS485_HALF_DUPLEX);

            ering.init();
            comm = new Wifibox_Comm(&ering);

            xTaskCreatePinnedToCore(
                this->rs485_worker_task,
                "rs485_task",
                4096, // Stack size
                this,
                10, // High priority
                NULL,
                1 // Core 1
            );
        }

        void Wifibox::rs485_worker_task(void *parameters)
        {
            Wifibox *instance = (Wifibox *)parameters;
            uint8_t c;

            while (true)
            {
                // int n = uart_read_bytes(UART_NUM_2, &c, 1, pdMS_TO_TICKS(1));
                while (uart_read_bytes(UART_NUM_2, &c, 1, 0) > 0)
                {
                    instance->comm->rx_write(c);
                }

                if (instance->comm->communication())
                {
                    uart_write_bytes(UART_NUM_2, (const char *)instance->comm->tx_data(), instance->comm->tx_length());
                    uart_wait_tx_done(UART_NUM_2, pdMS_TO_TICKS(1));
                }
                else
                {
                    vTaskDelay(pdMS_TO_TICKS(1));
                }
            }
        }

        void Wifibox::loop()
        {
            publish_sensor();
        }

        void Wifibox::register_sensor(IWifiBoxSensor *sensor)
        {
            sensors.emplace(sensor->get_key(), sensor);
        }

        void Wifibox::publish_sensor()
        {
            Event e;
            uint8_t has_sensor;
            while (ering.read(e))
            {
                has_sensor = 0;
                auto it = sensors.find(e.key);
                if (it != sensors.end())
                {
                    has_sensor = 1;
                    it->second->publish_val(e.value);
                }

                if (!has_sensor)
                {
                    ESP_LOGI("sensor", "event for %s with %s", e.key, e.value);
                }

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

        void Wifibox::push_json(const char *json)
        {
            comm->json_queue_push(json);
        }

    }
}
