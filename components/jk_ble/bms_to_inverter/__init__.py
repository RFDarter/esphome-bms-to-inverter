import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import ble_client

from esphome.components import bms_to_inverter
from esphome.const import (
    CONF_ID,
)

from esphome.core import coroutine_with_priority

jk_ble_ns = cg.esphome_ns.namespace("jk_ble")
BmsToInverterJKBle = jk_ble_ns.class_(
    "BmsToInverterJKBle",
    bms_to_inverter.BmsToInverter,
    cg.PollingComponent,
)

jk_bms_ble_ns = cg.esphome_ns.namespace("jk_bms_ble")
JkBmsBle = jk_bms_ble_ns.class_(
    "JkBmsBle", ble_client.BLEClientNode, cg.PollingComponent
)

CONFIG_SCHEMA = cv.All(
    bms_to_inverter.bms_to_inverter_schema(BmsToInverterJKBle).extend(
        {
            cv.Required(bms_to_inverter.CONF_BMS_ID): cv.use_id(JkBmsBle),
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
