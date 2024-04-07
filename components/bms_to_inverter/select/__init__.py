import esphome.codegen as cg
from esphome.components import select
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_OPTIONS,
    CONF_OPTIMISTIC,
    CONF_INITIAL_OPTION,
    CONF_RESTORE_VALUE,
    CONF_ENTITY_CATEGORY,
    ENTITY_CATEGORY_CONFIG,
)

from .. import CONF_BMS_TO_INVERTER_ID, BmsToInverter, bms_to_inverter_ns

DEPENDENCIES = ["bms_to_inverter"]

CODEOWNERS = ["@rfdarter"]

CONF_DEBUG_CHARGE_STATUS = "debug_charge_status"
CONF_USER_CHARGE_LOGIC = "user_charge_logic"
CONF_USER_CAN_PROTOCOL = "user_can_protocol"

DEBUG_CHARGE_STATUS_OPTIONS = ["AUTO", "WAIT", "BULK", "ABSORBTION", "FLOAT"]
CHARGE_LOGIC_OPTIONS = [
    "Rebulk voltage | Absorbtion Time",
    "Rebulk voltage | Absorbtion Cut-Off Current",
    "Bulk once per day | Absorbtion Time",
    "Bulk once per day | AAbsorbtion Cut-Off Current",
]
CAN_PROTOCOL_OPTIONS = ["PYLON 1.2", "PYLON +", "SMA", "VICTRON"]
BmsToInverterSelect = bms_to_inverter_ns.class_(
    "BmsToInverterSelect", select.Select, cg.Component
)

SELECTS = {CONF_DEBUG_CHARGE_STATUS, CONF_USER_CHARGE_LOGIC, CONF_USER_CAN_PROTOCOL}

BMS_TO_INVERTER_SELECT_SCHEMA = select.SELECT_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(BmsToInverterSelect),
        cv.Required(CONF_OPTIONS): cv.All(
            cv.ensure_list(cv.string_strict), cv.Length(min=1)
        ),
        cv.Optional(CONF_OPTIMISTIC, default=True): cv.boolean,
        cv.Optional(CONF_INITIAL_OPTION): cv.string_strict,
        cv.Optional(CONF_RESTORE_VALUE, default=True): cv.boolean,
        cv.Optional(
            CONF_ENTITY_CATEGORY, default=ENTITY_CATEGORY_CONFIG
        ): cv.entity_category,
    }
).extend(cv.COMPONENT_SCHEMA)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_BMS_TO_INVERTER_ID): cv.use_id(BmsToInverter),
        cv.Optional(CONF_DEBUG_CHARGE_STATUS): BMS_TO_INVERTER_SELECT_SCHEMA.extend(
            {
                cv.Optional(CONF_OPTIONS, default=DEBUG_CHARGE_STATUS_OPTIONS): cv.All(
                    cv.ensure_list(cv.string_strict), cv.Length(min=1)
                ),
                cv.Optional(CONF_INITIAL_OPTION, default="Auto"): cv.string_strict,
            }
        ),
        cv.Required(CONF_USER_CHARGE_LOGIC): BMS_TO_INVERTER_SELECT_SCHEMA.extend(
            {
                cv.Optional(CONF_OPTIONS, default=CHARGE_LOGIC_OPTIONS): cv.All(
                    cv.ensure_list(cv.string_strict), cv.Length(min=1)
                ),
                cv.Optional(
                    CONF_INITIAL_OPTION, default="Rebulk voltage | Absorbtion Time"
                ): cv.string_strict,
            }
        ),
        cv.Required(CONF_USER_CAN_PROTOCOL): BMS_TO_INVERTER_SELECT_SCHEMA.extend(
            {
                cv.Optional(CONF_OPTIONS, default=CAN_PROTOCOL_OPTIONS): cv.All(
                    cv.ensure_list(cv.string_strict), cv.Length(min=1)
                ),
                cv.Optional(CONF_INITIAL_OPTION, default="PYLON 1.2"): cv.string_strict,
            }
        ),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_BMS_TO_INVERTER_ID])
    for key in SELECTS:
        if key in config:
            conf = config[key]
            var = cg.new_Pvariable(conf[CONF_ID])
            await cg.register_component(var, conf)
            await select.register_select(var, conf, options=conf[CONF_OPTIONS])
            cg.add(var.set_optimistic(conf[CONF_OPTIMISTIC]))
            cg.add(var.set_initial_option(conf[CONF_INITIAL_OPTION]))

            if CONF_RESTORE_VALUE in conf:
                cg.add(var.set_restore_value(conf[CONF_RESTORE_VALUE]))

            cg.add(getattr(hub, f"set_{key}_select")(var))
