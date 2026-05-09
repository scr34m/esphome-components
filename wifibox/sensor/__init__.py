import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID

from .. import CONF_WIFIBOX_ID, CONF_KEY, Wifibox, key, wifibox_ns

AUTO_LOAD = ["wifibox"]

WifiboxSensor = wifibox_ns.class_("WifiboxSensor", sensor.Sensor, cg.Component)

CONFIG_SCHEMA = sensor.sensor_schema().extend(
    {
        cv.GenerateID(): cv.declare_id(WifiboxSensor),
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
    await sensor.register_sensor(var, config)
    wifibox = await cg.get_variable(config[CONF_WIFIBOX_ID])
    cg.add(wifibox.register_sensor(var))