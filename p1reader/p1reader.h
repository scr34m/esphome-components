#pragma once

#include "esphome.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/mqtt/mqtt_client.h"

namespace esphome {
    namespace p1reader {

        class IP1ReaderSensor {
            public:
                virtual const std::string& get_obis_code() const = 0;
                virtual void publish_val(const char* value) = 0;
        };

        class P1ReaderSensorBase : public IP1ReaderSensor {
            public:
                const std::string& get_obis_code() const override {
                    return _obis_code;
                }

            protected:
                std::string _obis_code;
        };

        class P1Reader : public PollingComponent, public uart::UARTDevice {
        public:
            P1Reader(uart::UARTComponent *parent): PollingComponent(), uart::UARTDevice(parent) {
            }

            void setup() override {
                set_update_interval(100);

                memset(_buffer, 0, BUF_SIZE);
                ESP_LOGI("setup", "Internal buffer size is %d", BUF_SIZE);
            }

            void update() override {
                uint8_t c;
                while (available()) {
                    read_byte(&c);

                    // telegram starts with identifier
                    if (c == '/') {
                        _crc = 0;
                        _telegram_status = 0;
                        _buffer_index = 0;
                    }

                    // calculate crc until end readed
                    if (_telegram_status == 0)
                        crc_16_update(c);

                    // overflow check
                    if (_buffer_index < BUF_SIZE - 2) {
                        _buffer[_buffer_index++] = (char)c;
                        _buffer[_buffer_index] = '\0';
                    } else {
                        _crc = 0;
                        _telegram_status = 0;
                        _buffer_index = 0;
                    }

                    // telegram end 4 more bytes for crc
                    if (c == '!') {
                        _telegram_status = _buffer_index;
                    }

                    if (_telegram_status != 0 && _telegram_status + 4 == _buffer_index) {
                        char *received_crc_str = &_buffer[_telegram_status]; 
                        uint16_t received_crc = (uint16_t)strtol(received_crc_str, NULL, 16);

                        // truncate garbage data 0-0:98.1.0 following 0-0:96.13
                        char *garbage_start = strstr(_buffer, "0-0:98.");
                        if (garbage_start != nullptr) {
                          *garbage_start = '\0';
                        }
                        
                        // publish raw message if any text sensor has this code
                        publish_sensor("raw", _buffer);

                        if (received_crc != _crc) {
                            ESP_LOGE("update", "Message CRC (%04x) != (%04x).", received_crc, _crc);
                        } else {
                            ESP_LOGI("update", "Message CRC OK (%04x) == (%04x).", received_crc, _crc);
                            parse_telegram();
                        }

                        _crc = 0;
                        _telegram_status = 0;
                        _buffer_index = 0;
                    }
                }
            }

            void crc_16_update(uint8_t a) {
                int i;
                _crc ^= a;
                for (i = 0; i < 8; ++i) {
                    if (_crc & 1)
                        _crc = (_crc >> 1) ^ 0xA001;
                    else
                        _crc = (_crc >> 1);
                }
            }

            void parse_telegram() {
                char *line = _buffer;
                char *next_line;

                while (line != NULL && *line != '\0') {
                    next_line = strchr(line, '\n'); 
                    if (next_line) {
                        *next_line = '\0';
                        next_line++;
                    }

                    process_line(line);

                    line = next_line;
                }                
            }

            void process_line(char* line) {
                char *bracket = strchr(line, '(');
                if (!bracket)
                  return;

                // obis code is before bracket, terminate on bracket
                *bracket = '\0';
                char *obis = line;

                // value starts after bracket
                char *value = bracket + 1;

                char *asterisk = strchr(value, '*');
                if (asterisk) {
                    *asterisk = '\0';
                } else {
                    char *closing_bracket = strchr(value, ')');
                    if (closing_bracket) *closing_bracket = '\0';
                }

                publish_sensor(obis, value);
            }

            void register_sensor(IP1ReaderSensor* sensor) {
                _sensors.emplace(sensor->get_obis_code(), sensor);
            }

            void publish_sensor(const char* code, char* value) {
                auto it = _sensors.find(code);
                if (it != _sensors.end()) {
                    it->second->publish_val(value);
                }
            }

        protected:
            char _buffer[BUF_SIZE];
            uint16_t _buffer_index;
            std::map<std::string, IP1ReaderSensor*> _sensors;
            uint16_t _crc;
            uint16_t _telegram_status;
        };
    }  // namespace p1reader
}  // namespace esphome
