#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "config.h"
#include "util.h"
#include "crc32.h"
#include "event.h"

#define JSON_QUEUE_MAX_ITEMS 10
#define JSON_QUEUE_MAX_CHARS 100

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

        enum BufferStatus
        {
            INACTIVE,
            PENDING,
            ACTIVE,
            COMPLETE
        };

        struct Buffer
        {
            uint8_t buffer[BUF_SIZE];
            uint32_t index = 0;
            uint8_t crc;
            uint8_t crc_disabled;
            BufferStatus status;
        };

        class Wifibox_Comm
        {
        public:
            uint8_t wifi_status;
            uint8_t internet_status;
            uint8_t mqtt_id_request_counter;
            uint8_t json_queue_count = 0;

            Wifibox_Comm(EventRing *event_ring);
            bool communication();
            const uint8_t *tx_data() const;
            uint16_t tx_length() const;
            void rx_write(uint8_t c);

            uint8_t json_queue_push(const char *s);
            uint8_t json_queue_shift(char *s);

        private:
            EventRing *ering;

            Buffer rx;
            uint16_t length;
            uint8_t buffer[BUF_SIZE];

            Buffer tx;
            uint16_t length_send;
            uint8_t buffer_send[BUF_SIZE];

            char json_queue[JSON_QUEUE_MAX_ITEMS][JSON_QUEUE_MAX_CHARS] = {0};

            bool packet_unescape(uint8_t *dst, uint16_t *length);
            uint32_t packet_crc32(const void *data, size_t size);
            void packet_escape(uint8_t *dst, uint16_t length);

            uint8_t get_wifi_signal();

            size_t packet_0a();
            size_t packet_0b();
            size_t packet_0c();
            size_t packet_0d();

            static uint16_t on_key_value(void *ctx, const char *key, const char *value);
        };
    }
}
