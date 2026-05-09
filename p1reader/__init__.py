import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import (
    CONF_UART_ID, CONF_ID
)

MULTI_CONF = True

DEPENDENCIES = ["uart"]

CONF_BUFFER_SIZE = "buffer_size"
CONF_P1READER_ID = "p1reader_id"
CONF_OBIS_CODE = "obis_code"

p1reader_ns = cg.esphome_ns.namespace("p1reader")
P1Reader = p1reader_ns.class_("P1Reader", cg.Component, uart.UARTDevice)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(P1Reader),
            cv.Optional(CONF_BUFFER_SIZE, default=256): cv.int_,
        }
    ).extend(uart.UART_DEVICE_SCHEMA)
)

async def to_code(config):
    uart_component = await cg.get_variable(config[CONF_UART_ID])
    var = cg.new_Pvariable(config[CONF_ID], uart_component)
    await cg.register_component(var, config)

    cg.add_define("BUF_SIZE", config[CONF_BUFFER_SIZE])

def obis_code(value):
    value = cv.string(value)
    return value