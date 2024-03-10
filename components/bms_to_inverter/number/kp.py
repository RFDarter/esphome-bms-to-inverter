from esphome import automation
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import number
from esphome.const import (
    CONF_ID,
    CONF_INITIAL_VALUE,
    CONF_LAMBDA,
    CONF_MAX_VALUE,
    CONF_MIN_VALUE,
    CONF_OPTIMISTIC,
    CONF_RESTORE_VALUE,
    CONF_STEP,
    CONF_SET_ACTION,
    CONF_ICON,
    ICON_EMPTY,
    CONF_UNIT_OF_MEASUREMENT,
    UNIT_VOLT,
    CONF_MODE,
    CONF_ENTITY_CATEGORY,
    ENTITY_CATEGORY_CONFIG,
)
from .. import bms_to_inverter_ns

BmsToInverterNumber = bms_to_inverter_ns.class_(
    "BmsToInverterNumber", number.Number, cg.PollingComponent
)

JK_NUMBER_SCHEMA = number.NUMBER_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(BmsToInverterNumber),
        cv.Optional(CONF_ICON, default=ICON_EMPTY): cv.icon,
        cv.Optional(CONF_STEP, default=0.01): cv.float_,
        cv.Optional(CONF_UNIT_OF_MEASUREMENT, default=UNIT_VOLT): cv.string_strict,
        cv.Optional(CONF_MODE, default="BOX"): cv.enum(number.NUMBER_MODES, upper=True),
        cv.Optional(CONF_MAX_VALUE, default=1): cv.float_,
        cv.Optional(CONF_MIN_VALUE, default=10): cv.float_,
        cv.Optional(CONF_INITIAL_VALUE): cv.float_,
        cv.Optional(
            CONF_ENTITY_CATEGORY, default=ENTITY_CATEGORY_CONFIG
        ): cv.entity_category,
    }
).extend(cv.COMPONENT_SCHEMA)

CONFIG_SCHEMA = {}

CONFIG_SCHEMA = cv.All(
    number.number_schema(BmsToInverterNumber)
    .extend(
        {
            cv.Required(CONF_MAX_VALUE): cv.float_,
            cv.Required(CONF_MIN_VALUE): cv.float_,
            cv.Required(CONF_STEP): cv.positive_float,
            cv.Optional(CONF_INITIAL_VALUE): cv.float_,
        }
    )
    .extend(cv.polling_component_schema("60s")),
    validate_min_max,
    validate,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await number.register_number(
        var,
        config,
        min_value=config[CONF_MIN_VALUE],
        max_value=config[CONF_MAX_VALUE],
        step=config[CONF_STEP],
    )

    cg.add(var.set_initial_value(config[CONF_INITIAL_VALUE]))
