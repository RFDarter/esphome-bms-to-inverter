import esphome.codegen as cg
from esphome.components import text_sensor
import esphome.config_validation as cv
from esphome.const import CONF_ICON, CONF_ID, ICON_TIMELAPSE

from . import CONF_BMS_TO_INVERTER_ID, BmsToInverter

DEPENDENCIES = ["bms_to_inverter"]

CODEOWNERS = ["@rfdarter"]

CONF_CHARGE_STATUS = "charge_status"

ICON_OPERATION_STATUS = "mdi:heart-pulse"

TEXT_SENSORS = [
    CONF_CHARGE_STATUS,
]

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_BMS_TO_INVERTER_ID): cv.use_id(BmsToInverter),
        cv.Required(CONF_CHARGE_STATUS): text_sensor.TEXT_SENSOR_SCHEMA.extend(
            {
                cv.GenerateID(): cv.declare_id(text_sensor.TextSensor),
                cv.Optional(CONF_ICON, default=ICON_OPERATION_STATUS): cv.icon,
            }
        ),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_BMS_TO_INVERTER_ID])
    for key in TEXT_SENSORS:
        if key in config:
            conf = config[key]
            sens = cg.new_Pvariable(conf[CONF_ID])

            cg.add(getattr(hub, f"set_{key}_text_sensor")(sens))
            await text_sensor.register_text_sensor(sens, conf)
