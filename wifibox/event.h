#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

namespace esphome
{
    namespace wifibox
    {
        struct Event
        {
            char key[32] = {};
            char value[128] = {};
        };

        struct EventRing
        {
            Event items[128];
            uint16_t w_index;
            uint16_t r_index;
            uint16_t mask;

            void init()
            {
                w_index = 0;
                r_index = 0;
                mask = 127;
            }

            void write(const char *key, const char *value)
            {
                strncpy(items[w_index].key, key, 32);
                strncpy(items[w_index].value, value, 128);
                w_index = w_index + 1;
                w_index = w_index & mask;
            }

            uint8_t read(Event &out)
            {
                if (r_index == w_index)
                    return 0;

                out = items[r_index];
                r_index = r_index + 1;
                r_index = r_index & mask;
                return 1;
            }
        };
    }
}