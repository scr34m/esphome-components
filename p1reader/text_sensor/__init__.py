import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import CONF_ID, CONF_TIMEOUT

from .. import CONF_P1READER_ID, CONF_OBIS_CODE, P1Reader, obis_code, p1reader_ns

AUTO_LOAD = ["p1reader"]

P1ReaderTextSensor = p1reader_ns.class_("P1ReaderTextSensor", text_sensor.TextSensor, cg.Component)

CONFIG_SCHEMA = text_sensor.text_sensor_schema().extend(
    {
        cv.GenerateID(): cv.declare_id(P1ReaderTextSensor),
        cv.GenerateID(CONF_P1READER_ID): cv.use_id(P1Reader),
        cv.Required(CONF_OBIS_CODE): obis_code,
    }
)

async def to_code(config):
    var = cg.new_Pvariable(
        config[CONF_ID],
        config[CONF_OBIS_CODE],
    )
    await cg.register_component(var, config)
    await text_sensor.register_text_sensor(var, config)
    p1reader = await cg.get_variable(config[CONF_P1READER_ID])
    cg.add(p1reader.register_sensor(var))
