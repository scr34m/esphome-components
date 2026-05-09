#pragma once

#include <stddef.h>
#include <stdint.h>

#include "util.h"

namespace esphome
{
    namespace wifibox
    {
        uint32_t crc32_reset();
        uint32_t crc32_add(uint32_t crc32, const void *data, size_t size);
        uint32_t crc32_finish(uint32_t crc32);
    }
}
