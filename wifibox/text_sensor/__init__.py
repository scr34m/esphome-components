import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import CONF_ID

from .. import CONF_WIFIBOX_ID, CONF_KEY, Wifibox, key, wifibox_ns

AUTO_LOAD = ["wifibox"]

WifiboxTextSensor = wifibox_ns.class_("WifiboxTextSensor", text_sensor.TextSensor, cg.Component)

CONFIG_SCHEMA = text_sensor.text_sensor_schema().extend(
    {
        cv.GenerateID(): cv.declare_id(WifiboxTextSensor),
        cv.GenerateID(CONF_WIFIBOX_ID): cv.use_id(Wifibox),
        cv.Required(CONF_KEY): key,
    }
)

async def to_code(config):
    var = cg.new_Pvariable(
        config[CONF_ID],
        config[CONF_KEY],
    )
    await cg.register_component(var, config)
    await text_sensor.register_text_sensor(var, config)
    wifibox = await cg.get_variable(config[CONF_WIFIBOX_ID])
    cg.add(wifibox.register_sensor(var))
