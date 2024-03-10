import esphome.codegen as cg
import esphome.config_validation as cv

from esphome.components import bms_to_inverter
from esphome.const import (
    CONF_ID,
)

from esphome.core import coroutine_with_priority

jk_ns = cg.esphome_ns.namespace("jk")
BmsToInverterJK = jk_ns.class_(
    "BmsToInverterJK",
    bms_to_inverter.BmsToInverter,
    cg.PollingComponent,
)

jk_modbus_ns = cg.esphome_ns.namespace("jk_modbus")
JkModbusDevice = jk_modbus_ns.class_("JkModbusDevice")

jk_bms_ns = cg.esphome_ns.namespace("jk_bms")
JkBms = jk_bms_ns.class_("JkBms", JkModbusDevice, cg.PollingComponent)

CONFIG_SCHEMA = cv.All(
    bms_to_inverter.bms_to_inverter_schema(BmsToInverterJK).extend(
        {
            cv.Required(bms_to_inverter.CONF_BMS_ID): cv.use_id(JkBms),
        }
    ),
)


@coroutine_with_priority(999.0)
async def to_code(config):
    bms_to_inverter_var = await cg.get_variable(config[bms_to_inverter.CONF_BMS_ID])
    var = cg.new_Pvariable(config[CONF_ID], bms_to_inverter_var)

    await cg.register_component(var, config)
    await bms_to_inverter.register_bms_to_inverter(
        var,
        config,
    )
