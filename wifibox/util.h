#pragma once

#include <stddef.h>
#include <stdint.h>

namespace esphome
{
    namespace wifibox
    {
        typedef uint16_t (*json_key_value_cb)(void *ctx, const char *key, const char *value);

        uint8_t bit_reverse(uint8_t x);
        uint32_t bit_reverse32(uint32_t x);
        uint8_t crc8_add(uint8_t crc, uint8_t data);
        void int_to_buf(uint32_t value, uint8_t *data);
        uint16_t json_parser(char *buf, uint16_t length, void *ctx, json_key_value_cb on_key_value);
    }
}
