import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import (
    CONF_UART_ID, CONF_ID
)

MULTI_CONF = True

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["sensor"]

CONF_WIFIBOX_ID = "wifibox_id"
CONF_BUFFER_SIZE = "buffer_size"
CONF_MQTT_ID = "mqtt_id"
CONF_WIFI_VER = "wifi_ver"
CONF_KEY = "key"

wifibox_ns = cg.esphome_ns.namespace("wifibox")
Wifibox = wifibox_ns.class_("Wifibox", cg.Component, uart.UARTDevice)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(Wifibox),
            cv.Optional(CONF_BUFFER_SIZE, default=256): cv.int_,
            cv.Optional(CONF_MQTT_ID, default="ABCD1234"): cv.string,
            cv.Optional(CONF_WIFI_VER, default="v1.15"): cv.string,
        }
    ).extend(uart.UART_DEVICE_SCHEMA)
)

async def to_code(config):
    uart_component = await cg.get_variable(config[CONF_UART_ID])
    var = cg.new_Pvariable(config[CONF_ID], uart_component)
    await cg.register_component(var, config)

    cg.add_define("BUF_SIZE", config[CONF_BUFFER_SIZE])
    cg.add_define("_MQTT_ID", config[CONF_MQTT_ID])
    cg.add_define("_WIFI_VER", config[CONF_WIFI_VER])

def key(value):
    value = cv.string(value)
    return value