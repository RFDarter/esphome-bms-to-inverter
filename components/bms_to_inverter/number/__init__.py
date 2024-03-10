import esphome.codegen as cg
from esphome.components import number
import esphome.config_validation as cv
from esphome.const import (
    CONF_ENTITY_CATEGORY,
    CONF_ICON,
    CONF_ID,
    CONF_MAX_VALUE,
    CONF_MIN_VALUE,
    CONF_MODE,
    CONF_STEP,
    CONF_INITIAL_VALUE,
    CONF_UNIT_OF_MEASUREMENT,
    ENTITY_CATEGORY_CONFIG,
    ICON_EMPTY,
    UNIT_AMPERE,
    UNIT_EMPTY,
    UNIT_VOLT,
    UNIT_HOUR,
    CONF_RESTORE_VALUE,
)

from .. import CONF_BMS_TO_INVERTER_ID, BmsToInverter, bms_to_inverter_ns

DEPENDENCIES = ["bms_to_inverter"]

CODEOWNERS = ["@rfdarter"]

DEFAULT_STEP = 1

CONF_USER_CHARGE_VOLTAGE = "user_charge_voltage"
CONF_USER_FLOAT_VOLTAGE = "user_float_voltage"
CONF_USER_REBULK_VOLTAGE = "user_rebulk_voltage"
CONF_USER_ABSORBTION_TIME = "user_absorbtion_time"
CONF_USER_FLOAT_TIME = "user_float_time"

CONF_USER_MAX_CHARGE_CURRENT = "user_max_charge_current"
CONF_USER_MAX_DISCHARGE_CURRENT = "user_max_discharge_current"

BmsToInverterNumber = bms_to_inverter_ns.class_(
    "BmsToInverterNumber", number.Number, cg.Component
)

NUMBERS = {
    CONF_USER_CHARGE_VOLTAGE,
    CONF_USER_FLOAT_VOLTAGE,
    CONF_USER_REBULK_VOLTAGE,
    CONF_USER_ABSORBTION_TIME,
    CONF_USER_FLOAT_TIME,
    CONF_USER_MAX_CHARGE_CURRENT,
    CONF_USER_MAX_DISCHARGE_CURRENT,
}

BMS_TO_INVERTER_NUMBER_SCHEMA = number.NUMBER_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(BmsToInverterNumber),
        cv.Optional(CONF_ICON, default=ICON_EMPTY): cv.icon,
        cv.Optional(CONF_STEP, default=0.01): cv.float_,
        cv.Optional(CONF_INITIAL_VALUE, default=0.0): cv.float_,
        cv.Optional(CONF_RESTORE_VALUE, default=True): cv.boolean,
        cv.Optional(CONF_UNIT_OF_MEASUREMENT, default=UNIT_VOLT): cv.string_strict,
        cv.Optional(CONF_MODE, default="BOX"): cv.enum(number.NUMBER_MODES, upper=True),
        cv.Optional(
            CONF_ENTITY_CATEGORY, default=ENTITY_CATEGORY_CONFIG
        ): cv.entity_category,
    }
).extend(cv.COMPONENT_SCHEMA)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_BMS_TO_INVERTER_ID): cv.use_id(BmsToInverter),
        cv.Required(CONF_USER_CHARGE_VOLTAGE): BMS_TO_INVERTER_NUMBER_SCHEMA.extend(
            {
                cv.Optional(CONF_MIN_VALUE, default=0.0): cv.float_,
                cv.Optional(CONF_MAX_VALUE, default=5.0): cv.float_,
                cv.Optional(CONF_STEP, default=0.01): cv.float_,
                cv.Optional(CONF_INITIAL_VALUE, default=3.5): cv.float_,
            }
        ),
        cv.Required(CONF_USER_FLOAT_VOLTAGE): BMS_TO_INVERTER_NUMBER_SCHEMA.extend(
            {
                cv.Optional(CONF_MIN_VALUE, default=0.0): cv.float_,
                cv.Optional(CONF_MAX_VALUE, default=5.0): cv.float_,
                cv.Optional(CONF_STEP, default=0.01): cv.float_,
                cv.Optional(CONF_INITIAL_VALUE, default=3.35): cv.float_,
            }
        ),
        cv.Required(CONF_USER_REBULK_VOLTAGE): BMS_TO_INVERTER_NUMBER_SCHEMA.extend(
            {
                cv.Optional(CONF_MIN_VALUE, default=0.0): cv.float_,
                cv.Optional(CONF_MAX_VALUE, default=5.0): cv.float_,
                cv.Optional(CONF_STEP, default=0.01): cv.float_,
                cv.Optional(CONF_INITIAL_VALUE, default=3.34): cv.float_,
            }
        ),
        cv.Required(CONF_USER_ABSORBTION_TIME): BMS_TO_INVERTER_NUMBER_SCHEMA.extend(
            {
                cv.Optional(CONF_MIN_VALUE, default=0): cv.float_,
                cv.Optional(CONF_MAX_VALUE, default=24): cv.float_,
                cv.Optional(CONF_STEP, default=0.1): cv.float_,
                cv.Optional(CONF_INITIAL_VALUE, default=1.0): cv.float_,
                cv.Optional(
                    CONF_UNIT_OF_MEASUREMENT, default=UNIT_HOUR
                ): cv.string_strict,
            }
        ),
        cv.Required(CONF_USER_FLOAT_TIME): BMS_TO_INVERTER_NUMBER_SCHEMA.extend(
            {
                cv.Optional(CONF_MIN_VALUE, default=0): cv.float_,
                cv.Optional(CONF_MAX_VALUE, default=24): cv.float_,
                cv.Optional(CONF_STEP, default=1.0): cv.float_,
                cv.Optional(CONF_INITIAL_VALUE, default=6.0): cv.float_,
                cv.Optional(
                    CONF_UNIT_OF_MEASUREMENT, default=UNIT_HOUR
                ): cv.string_strict,
            }
        ),
        cv.Required(CONF_USER_MAX_CHARGE_CURRENT): BMS_TO_INVERTER_NUMBER_SCHEMA.extend(
            {
                cv.Optional(CONF_MIN_VALUE, default=0): cv.float_,
                cv.Optional(CONF_MAX_VALUE, default=300): cv.float_,
                cv.Optional(CONF_STEP, default=1.0): cv.float_,
                cv.Optional(CONF_INITIAL_VALUE, default=100): cv.float_,
                cv.Optional(
                    CONF_UNIT_OF_MEASUREMENT, default=UNIT_AMPERE
                ): cv.string_strict,
            }
        ),
        cv.Required(
            CONF_USER_MAX_DISCHARGE_CURRENT
        ): BMS_TO_INVERTER_NUMBER_SCHEMA.extend(
            {
                cv.Optional(CONF_MIN_VALUE, default=0): cv.float_,
                cv.Optional(CONF_MAX_VALUE, default=300): cv.float_,
                cv.Optional(CONF_STEP, default=1.0): cv.float_,
                cv.Optional(CONF_INITIAL_VALUE, default=100): cv.float_,
                cv.Optional(
                    CONF_UNIT_OF_MEASUREMENT, default=UNIT_AMPERE
                ): cv.string_strict,
            }
        ),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_BMS_TO_INVERTER_ID])
    for key in NUMBERS:
        if key in config:
            conf = config[key]
            var = cg.new_Pvariable(conf[CONF_ID])
            await cg.register_component(var, conf)
            await number.register_number(
                var,
                conf,
                min_value=conf[CONF_MIN_VALUE],
                max_value=conf[CONF_MAX_VALUE],
                step=conf[CONF_STEP],
            )
            cg.add(var.set_initial_value(conf[CONF_INITIAL_VALUE]))
            cg.add(var.set_restore_value(conf[CONF_RESTORE_VALUE]))
            cg.add(getattr(hub, f"set_{key}_number")(var))
