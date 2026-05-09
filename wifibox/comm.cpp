#include "esphome.h"
#include "comm.h"

namespace esphome
{
    namespace wifibox
    {
        uint8_t Wifibox_Comm::get_wifi_signal()
        {
            uint8_t b = 0x19;
            if ((internet_status & 1) != 0)
                b = b | 0x20;
            return b;
        }

        Wifibox_Comm::Wifibox_Comm(EventRing *event_ring)
        {
            ering = event_ring;
            ering->init();

            comm_status = IDLE;
            tx.status = INACTIVE;
            internet_status = 0;
            mqtt_id_request_counter = 0;
        }

        void Wifibox_Comm::communication()
        {
            if (comm_status == SEND)
            {
                uint8_t rc = packet_write(buffer_send, length_send);
                if (rc != 1)
                    return;

                comm_status = IDLE;
                return;
            }

            if (comm_status == IDLE)
            {
                uint8_t rc = packet_read((uint8_t *)&buffer, &length);
                if (rc == 0x3)
                {
                    comm_status = PARSE;
                    return;
                }
            }

            if (comm_status != PARSE)
                return;

            if (buffer[0] != 0x80 || buffer[1] != 0x71)
            {
                comm_status = IDLE;
                return;
            }

            uint32_t crc;
            memset(buffer_send, 0, BUF_SIZE);
            switch (buffer[7])
            {
            case 0x0a: // boiler sends data
                length_send = packet_0a();
                comm_status = SEND;
                return;
            case 0x0b: // boiler request data
                length_send = packet_0b();
                comm_status = SEND;
                return;
            case 0x0c: // boiler sends data
                length_send = packet_0c();
                comm_status = SEND;
                return;
            case 0x0d: // boiler request data
                length_send = packet_0d();
                comm_status = SEND;
                return;
            }
        }

        uint16_t Wifibox_Comm::on_key_value(void *ctx, const char *key, const char *value)
        {
            Wifibox_Comm* self = static_cast<Wifibox_Comm*>(ctx);
            self->ering->write(key, value);
            return 0;
        }

        size_t Wifibox_Comm::packet_0a()
        {
            uint32_t crc;

            json_parser((char*)buffer + 10, buffer[2] - 3, this, on_key_value);

            buffer_send[0] = 0x71;
            buffer_send[1] = 0x80;
            buffer_send[2] = 5;
            buffer_send[7] = 0xa;
            buffer_send[10] = internet_status;
            buffer_send[11] = get_wifi_signal();
            crc = packet_crc32(buffer_send + 7, buffer[2]);
            int_to_buf(crc, buffer_send + 3);
            return 0xc - 1;
        }

        size_t Wifibox_Comm::packet_0b()
        {
            uint32_t crc;

            uint16_t seq_number = buffer[8] | (buffer[9] << 8);
            size_t l = 0;

            if (mqtt_id_request_counter < 10)
            {
                const char json[] = "{\"_MQTT_ID\":\"" _MQTT_ID "\",\"_WIFI_VER\":\"" _WIFI_VER "\"}";
                l = sizeof(json) - 1;
                for (size_t i = 0; i <= l; i++)
                    buffer_send[12 + i] = json[i];
                mqtt_id_request_counter = mqtt_id_request_counter + 1;
            }

            buffer_send[0] = 0x71;
            buffer_send[1] = 0x80;
            buffer_send[2] = 5 + l;
            buffer_send[7] = 0xb;
            buffer_send[8] = seq_number & 0xFF;
            buffer_send[9] = (seq_number >> 8) & 0xFF;
            buffer_send[10] = internet_status;
            buffer_send[11] = get_wifi_signal();
            crc = packet_crc32(buffer_send + 7, buffer_send[2]);
            int_to_buf(crc, buffer_send + 3);
            return 0xc + l - 1;
        }

        size_t Wifibox_Comm::packet_0c()
        {
            uint32_t crc;
            buffer_send[0] = 0x71;
            buffer_send[1] = 0x80;
            buffer_send[2] = 5;
            buffer_send[7] = 0xc;
            buffer_send[10] = internet_status;
            buffer_send[11] = get_wifi_signal();
            crc = packet_crc32(buffer_send + 7, buffer[2]);
            int_to_buf(crc, buffer_send + 3);
            return 0xc - 1;
        }

        size_t Wifibox_Comm::packet_0d()
        {
            uint32_t crc;
            buffer_send[0] = 0x71;
            buffer_send[1] = 0x80;
            buffer_send[2] = 5;
            buffer_send[7] = 0xd;
            buffer_send[10] = internet_status;
            buffer_send[11] = get_wifi_signal();
            crc = packet_crc32(buffer_send + 7, buffer[2]);
            int_to_buf(crc, buffer_send + 3);
            return 0xc - 1;
        }

        uint8_t Wifibox_Comm::read(void)
        {
            if (tx.status == PENDING)
                return tx.read();

            return 0;
        }

        uint8_t Wifibox_Comm::available(void)
        {
            if (tx.status == PENDING)
            {
                if (tx.read_index != tx.write_index)
                    return 1;

                tx.status = ACTIVE;
            }

            return 0;
        }

        void Wifibox_Comm::write(uint8_t c)
        {
            if (c == 0xBB)
            {
                rx.write_index = 0;
                rx.read_index = 0;
                rx.status = ACTIVE;
                rx.crc = 0;
                rx.crc_disabled = 0;
            }

            rx.write(c);

            if (rx.status == ACTIVE)
            {
                // 1st check, readed at least 3 bytes: 0xBB, source, target
                // 2nd check, is not a checksum marker: 0xCC or 0xCD
                if (rx.write_index < 3 || (c >> 1 != 0x66))
                {
                    if (rx.crc_disabled == 0)
                    {
                        rx.crc = crc8_add(rx.crc, c);
                    }
                }
                else
                {
                    rx.crc_disabled = 1;
                }

                // 1st check, readed at least 4 bytes and current one is 0xEE
                // 2nd check, CRC calculation finished
                if (((4 < rx.write_index) && (c == 0xee)) && (rx.crc_disabled == 1))
                {
                    c = rx.buffer[rx.write_index - 2]; // received crc8 value
                    if ((rx.buffer[rx.write_index - 3] & 1) != 0)
                    {
                        // if it is is 0xCD
                        c = c | 0x80;
                    }
                    if (rx.crc == c)
                    {
                        rx.status = CRC_OK;
                    }
                    else
                    {
                        rx.status = CRC_ERROR;
                    }
                }
            }
        }

        uint8_t Wifibox_Comm::packet_write(uint8_t *data, uint16_t length)
        {
            if (tx.status != PENDING && tx.status == ACTIVE)
            {
                tx.status = INACTIVE;
                return 1;
            }

            if (tx.status != INACTIVE)
                return 0;

            if (length == 0)
                return 0;

            uint8_t b;
            tx.write_index = 0;
            tx.write(0xBB);
            for (uint16_t i = 0; i <= length; i++)
            {
                b = data[i];
                switch (b)
                {
                case 0xAA:
                    tx.write(0xAA);
                    tx.write(0x04);
                    break;
                case 0xBB:
                    tx.write(0xAA);
                    tx.write(0x00);
                    break;
                case 0xCC:
                    tx.write(0xAA);
                    tx.write(0x01);
                    break;
                case 0xCD:
                    tx.write(0xAA);
                    tx.write(0x02);
                    break;
                case 0xEE:
                    tx.write(0xAA);
                    tx.write(0x03);
                    break;
                default:
                    tx.write(b);
                    break;
                }
            }

            uint8_t crc = 0;
            for (size_t i = 0; i < tx.write_index; i++)
            {
                crc = crc8_add(crc, tx.buffer[i]);
            }

            tx.write((crc & 0x80) ? 0xCD : 0xCC);
            tx.write(crc & 0x7F);

            tx.write(0xEE);

            tx.status = PENDING;
            tx.read_index = 0;
            return 0;
        }

        uint8_t Wifibox_Comm::packet_read(uint8_t *dst, uint16_t *length)
        {
            if (rx.status == INACTIVE)
                return 1;

            if (rx.status == PENDING)
                return 0x10;

            if (rx.status == ACTIVE)
                return 2;

            if (rx.status != CRC_OK)
            {
                if (rx.status != CRC_ERROR)
                    return 0;

                rx.status = INACTIVE;
                return 0x11;
            }

            uint8_t b;
            rx.status = INACTIVE;
            *length = 0;
            rx.read_index = 1; // skip 0xBB

            do
            {
                // 1st check, read and write index is the same position
                // 2nd check, is a checksum marker: 0xCC or 0xCD
                if ((rx.read_index == rx.write_index) || (rx.buffer[rx.read_index] >> 1 == 0x66))
                    return 3;

                b = rx.read();
                if (b == 0xAA)
                {
                    b = rx.read();
                    switch (b)
                    {
                    case 0:
                        b = 0xBB;
                        break;
                    case 1:
                        b = 0xCC;
                        break;
                    case 2:
                        b = 0xCD;
                        break;
                    case 3:
                        b = 0xEE;
                        break;
                    case 4:
                        b = 0xAA;
                        break;
                    }
                }
                dst[*length] = b;
                *length = *length + 1;
            } while (true);

            return 0x10;
        }

        uint32_t Wifibox_Comm::packet_crc32(const void *data, size_t size)
        {
            uint32_t crc = esphome::wifibox::crc32_reset();
            crc = esphome::wifibox::crc32_add(crc, data, size);
            return esphome::wifibox::crc32_finish(crc);
        }

        uint8_t *Wifibox_Comm::debug_get_buffer()
        {
            return (uint8_t *)&buffer;
        }

        uint8_t *Wifibox_Comm::debug_get_buffer_send()
        {
            return (uint8_t *)&buffer_send;
        }

        uint8_t *Wifibox_Comm::debug_get_ring_buffer()
        {
            return (uint8_t *)&rx.buffer;
        }

        uint8_t *Wifibox_Comm::debug_get_ring_buffer_send()
        {
            return (uint8_t *)&tx.buffer;
        }
    }
}