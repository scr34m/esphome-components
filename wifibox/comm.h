#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "config.h"
#include "util.h"
#include "crc32.h"
#include "event.h"

namespace esphome
{
    namespace wifibox
    {
        enum CommunicationStatus
        {
            READ,
            PROCESS,
            WRITE
        };

        enum RingBufferStatus
        {
            INACTIVE,
            PENDING,
            ACTIVE,
            CRC_OK,
            CRC_ERROR
        };

        struct RingBuffer
        {
            uint8_t buffer[BUF_SIZE];
            uint32_t write_index = 0;
            uint32_t read_index = 0;
            uint32_t size = BUF_SIZE;
            uint32_t mask = BUF_SIZE - 1;
            RingBufferStatus status;
            uint8_t crc;
            uint8_t crc_disabled;

            RingBufferStatus write_status;

            void write(uint8_t b)
            {
                buffer[write_index] = b;
                write_index = write_index + 1;
                write_index = write_index & mask;
            }

            uint8_t read()
            {
                uint8_t b = buffer[read_index];
                read_index = read_index + 1;
                read_index = read_index & mask;
                return b;
            }
        };

        class Wifibox_Comm
        {
        public:
            uint8_t wifi_status;
            uint8_t internet_status;
            uint8_t mqtt_id_request_counter;

            Wifibox_Comm(EventRing *event_ring);
            bool communication();
            uint8_t read(void);
            uint8_t available(void);
            void write(uint8_t c);

        private:
            EventRing *ering;
            CommunicationStatus comm_status;
            RingBuffer rx;
            uint16_t length;
            uint8_t buffer[BUF_SIZE];

            RingBuffer tx;
            uint16_t length_send;
            uint8_t buffer_send[BUF_SIZE];

            uint8_t packet_unescape(uint8_t *dst, uint16_t *length);
            uint32_t packet_crc32(const void *data, size_t size);
            uint8_t packet_escape(uint8_t *dst, uint16_t length);

            uint8_t get_wifi_signal();

            size_t packet_0a();
            size_t packet_0b();
            size_t packet_0c();
            size_t packet_0d();

            static uint16_t on_key_value(void *ctx, const char *key, const char *value);
        };
    }
}
