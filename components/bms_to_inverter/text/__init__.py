import esphome.codegen as cg
from esphome.components import text
import esphome.config_validation as cv
from esphome.const import (
    CONF_ENTITY_CATEGORY,
    CONF_ID,
    CONF_INITIAL_VALUE,
    ENTITY_CATEGORY_CONFIG,
    CONF_MODE,
    CONF_MIN_LENGTH,
    CONF_MAX_LENGTH,
)

from .. import CONF_BMS_TO_INVERTER_ID, BmsToInverter, bms_to_inverter_ns

DEPENDENCIES = ["bms_to_inverter"]

CODEOWNERS = ["@rfdarter"]

CONF_USER_BATTERY_NAME = "user_battery_name"

TextSaverBase = bms_to_inverter_ns.class_("TemplateTextSaverBase")
TextSaverTemplate = bms_to_inverter_ns.class_("TextSaver", TextSaverBase)

BmsToInverterText = bms_to_inverter_ns.class_(
    "BmsToInverterText", text.Text, cg.Component
)

TEXTS = {
    CONF_USER_BATTERY_NAME,
}

BMS_TO_INVERTER_TEXT_SCHEMA = text.TEXT_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(BmsToInverterText),
        cv.Optional(CONF_INITIAL_VALUE, default=""): cv.string,
        cv.Optional(CONF_MIN_LENGTH, default=0): cv.int_range(min=0, max=255),
        cv.Optional(CONF_MAX_LENGTH, default=255): cv.int_range(min=0, max=255),
        cv.Optional(CONF_MODE, default="TEXT"): cv.enum(text.TEXT_MODES),
        cv.Optional(
            CONF_ENTITY_CATEGORY, default=ENTITY_CATEGORY_CONFIG
        ): cv.entity_category,
    }
).extend(cv.COMPONENT_SCHEMA)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_BMS_TO_INVERTER_ID): cv.use_id(BmsToInverter),
        cv.Optional(CONF_USER_BATTERY_NAME): BMS_TO_INVERTER_TEXT_SCHEMA.extend(
            {
                cv.Optional(
                    CONF_INITIAL_VALUE, default="Bms to Inverter Battery"
                ): cv.string,
            }
        ),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_BMS_TO_INVERTER_ID])
    for key in TEXTS:
        if key in config:
            conf = config[key]
            var = cg.new_Pvariable(conf[CONF_ID])
            await cg.register_component(var, conf)
            await text.register_text(
                var,
                conf,
            )
            cg.add(var.set_initial_value(conf[CONF_INITIAL_VALUE]))
            args = cg.TemplateArguments(conf[CONF_MAX_LENGTH])
            saver = TextSaverTemplate.template(args).new()
            cg.add(var.set_value_saver(saver))
            cg.add(getattr(hub, f"set_{key}_text")(var))
