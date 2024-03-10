import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import switch
from esphome.const import (
    CONF_ASSUMED_STATE,
    CONF_ID,
    CONF_ICON,
    CONF_OPTIMISTIC,
    CONF_ENTITY_CATEGORY,
    ENTITY_CATEGORY_CONFIG,
    CONF_RESTORE_MODE,
)
from .. import CONF_BMS_TO_INVERTER_ID, BmsToInverter, bms_to_inverter_ns


DEPENDENCIES = ["bms_to_inverter"]

CODEOWNERS = ["@rfdarter"]

CONF_TEST_SWITCH = "test_switch"

ICON_CHARGING = "mdi:battery-charging-50"
ICON_DISCHARGING = "mdi:battery-charging-50"
ICON_BALANCER = "mdi:seesaw"
ICON_EMERGENCY = "mdi:exit-run"
ICON_HEATING = "mdi:radiator"
ICON_DISABLE_TEMPERATURE_SENSORS = "mdi:thermometer-off"
ICON_DISPLAY_ALWAYS_ON = "mdi:led-on"
ICON_SMART_SLEEP = "mdi:sleep"
ICON_TIMED_STORED_DATA = "mdi:calendar-clock"
ICON_DISABLE_PCL_MODULE = "mdi:power-plug-off"
ICON_CHARGING_FLOAT_MODE = "mdi:battery-charging-80"

SWITCHES = {
    CONF_TEST_SWITCH,
}

BmsToInverterSwitch = bms_to_inverter_ns.class_(
    "BmsToInverterSwitch", switch.Switch, cg.Component
)

BMS_TO_INVERTER_SWITCH_SCHEMA = switch.SWITCH_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(BmsToInverterSwitch),
        cv.Optional(CONF_OPTIMISTIC, default=True): cv.boolean,
        cv.Optional(CONF_ASSUMED_STATE, default=False): cv.boolean,
        cv.Optional(CONF_RESTORE_MODE, default="RESTORE_DEFAULT_OFF"): cv.enum(
            switch.RESTORE_MODES, upper=True, space="_"
        ),
        cv.Optional(
            CONF_ENTITY_CATEGORY, default=ENTITY_CATEGORY_CONFIG
        ): cv.entity_category,
    }
).extend(cv.COMPONENT_SCHEMA)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_BMS_TO_INVERTER_ID): cv.use_id(BmsToInverter),
        cv.Optional(CONF_TEST_SWITCH): BMS_TO_INVERTER_SWITCH_SCHEMA.extend(
            {
                cv.Optional(CONF_ICON, default=ICON_CHARGING_FLOAT_MODE): cv.icon,
            }
        ),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_BMS_TO_INVERTER_ID])
    for key in SWITCHES:
        if key in config:
            conf = config[key]
            var = cg.new_Pvariable(conf[CONF_ID])
            await cg.register_component(var, conf)
            await switch.register_switch(var, conf)
            cg.add(var.set_optimistic(conf[CONF_OPTIMISTIC]))
            cg.add(var.set_assumed_state(conf[CONF_ASSUMED_STATE]))
            cg.add(getattr(hub, f"set_{key}_switch")(var))
