# pylint: skip-file
# pylint: disable=disallowed-name
# pylint: disable-msg-cat=WCREFI
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_TYPE
from esphome.cpp_generator import MockObjClass
from esphome.core import CORE, coroutine_with_priority
from esphome.components.canbus import CONF_CANBUS_ID, CanbusComponent

from esphome.components import mqtt

IS_PLATFORM_COMPONENT = True

AUTO_LOAD = ["canbus", "number", "text", "select", "switch", "bms_to_inverter"]

CODEOWNERS = ["@rfdarter"]

MULTI_CONF = True

CONF_BMS_TO_INVERTER_ID = "bms_to_inverter_id"
CONF_DEVICE_ROLE = "device_role"
CONF_BMS_ID = "bms_id"
CONF_INVERTER_TYPE = "inverter_type"
CONF_INVERTER_CONNECTION = "inverter_connection"
CONF_MASTER_CONNECTION = "master_connection"
CONF_MASTER_DOMAIN = "master_domain"
CONF_MQTT_TOPIC = "mqtt_topic"

CONF_BATTERY_NAME = "battery_name"

TEXTS = [CONF_BATTERY_NAME]

DEFAULT_MQTT_TOPIC = "enphase/battery"

CustomMQTTDevice = mqtt.mqtt_ns.class_("CustomMQTTDevice")

bms_to_inverter_ns = cg.esphome_ns.namespace("bms_to_inverter")
BmsToInverter = bms_to_inverter_ns.class_(
    "BmsToInverter", CustomMQTTDevice, cg.PollingComponent
)

InverterType = bms_to_inverter_ns.enum("InverterType")
INVERTER_TYPE_OPTIONS = {
    "generic": InverterType.INVERTER_TYPE_GENERIC,
    "victron": InverterType.INVERTER_TYPE_VICTRON,
}

InverterConnectionType = bms_to_inverter_ns.enum("InverterConnectionType")
INVERTER_CONNECTION_TYPE_OPTIONS = {
    "serial": InverterConnectionType.INVERTER_CONNECTION_TYPE_SERIAL,
    "can": InverterConnectionType.INVERTER_CONNECTION_TYPE_CAN,
    "mqtt": InverterConnectionType.INVERTER_CONNECTION_TYPE_MQTT,
}

MasterConnectionType = bms_to_inverter_ns.enum("MasterConnectionType")
MASTER_CONNECTION_TYPE_OPTIONS = {
    "serial": MasterConnectionType.MASTER_CONNECTION_TYPE_SERIAL,
    "can": MasterConnectionType.MASTER_CONNECTION_TYPE_CAN,
    "wifi": MasterConnectionType.MASTER_CONNECTION_TYPE_WIFI,
}

DeviceRole = bms_to_inverter_ns.enum("DeviceRole")
DEVICE_ROLE_OPTIONS = {
    "master": DeviceRole.DEVICE_ROLE_MASTER,
    "slave": DeviceRole.DEVICE_ROLE_SLAVE,
}


def validate_BmsToInverter(config):
    if config[CONF_DEVICE_ROLE] == "master":
        if CONF_INVERTER_CONNECTION not in config:
            raise cv.Invalid(
                f"'{CONF_DEVICE_ROLE}' is 'master'. You need to provide '{CONF_INVERTER_CONNECTION}'"
            )
        if CONF_MASTER_CONNECTION in config:
            raise cv.Invalid(
                f"'{CONF_DEVICE_ROLE}' is 'master'. You can not provide '{CONF_MASTER_CONNECTION}'"
            )
        if (
            config[CONF_INVERTER_TYPE] != "victron"
            and config[CONF_INVERTER_CONNECTION][CONF_TYPE] != "can"
        ):
            raise cv.Invalid(
                f"'{config[CONF_INVERTER_CONNECTION][CONF_TYPE]}' can only be used if the '{CONF_INVERTER_TYPE}' is victron"
            )
    elif config[CONF_DEVICE_ROLE] == "slave":
        if CONF_MASTER_CONNECTION not in config:
            raise cv.Invalid(
                f"'{CONF_DEVICE_ROLE}' is 'slave'. You need to provide '{CONF_MASTER_CONNECTION}'"
            )
        if CONF_INVERTER_CONNECTION in config:
            raise cv.Invalid(
                f"'{CONF_DEVICE_ROLE}' is 'slave'. You can not provide '{CONF_INVERTER_CONNECTION}'"
            )

    # keys = [CONF_FLOAT_VOLTAGE_ID, CONF_FLOAT_TIME_ID]
    # number = sum(k in keys for k in config)
    # if number != 0 and number != len(keys):
    #     raise cv.Invalid(f"Must specify either none or all of {', '.join(keys)}.")
    return config


def validate_master_connection(config):
    if config[CONF_TYPE] == "serial":
        ...
    elif config[CONF_TYPE] == "can":
        if CONF_CANBUS_ID not in config:
            raise cv.Invalid(
                f"When using connection type '{config[CONF_TYPE]}' you need to provide '{CONF_CANBUS_ID}'."
            )
    elif config[CONF_TYPE] == "wifi":
        if CONF_MASTER_DOMAIN not in config:
            raise cv.Invalid(
                f"When using connection type '{config[CONF_TYPE]}' you need to provide '{CONF_MASTER_DOMAIN}'."
            )
    if CONF_MASTER_DOMAIN in config and config[CONF_TYPE] != "wifi":
        raise cv.Invalid(
            f"Connection type is not 'wifi' you should not provide '{CONF_MASTER_DOMAIN}'."
        )
    if CONF_CANBUS_ID in config and config[CONF_TYPE] != "can":
        raise cv.Invalid(
            f"Connection type is not 'can' you should not provide '{CONF_CANBUS_ID}'."
        )
    return config


def validate_inverter_connection(config):
    if config[CONF_TYPE] == "serial":
        ...
    elif config[CONF_TYPE] == "can":
        if CONF_CANBUS_ID not in config:
            raise cv.Invalid(
                f"When using connection type '{config[CONF_TYPE]}' you need to provide '{CONF_CANBUS_ID}'."
            )
        if CONF_MQTT_TOPIC in config:
            raise cv.Invalid(
                f"When using connection type '{config[CONF_TYPE]}' you can not provide '{CONF_MQTT_TOPIC}'."
            )
    elif config[CONF_TYPE] == "mqtt":
        if "mqtt" not in CORE.loaded_integrations:
            raise cv.Invalid("This option requires component mqtt")

    if CONF_CANBUS_ID in config and config[CONF_TYPE] != "can":
        raise cv.Invalid(
            f"Connection type is not 'can' you can not provide '{CONF_CANBUS_ID}'."
        )
    if CONF_MQTT_TOPIC in config and config[CONF_TYPE] != "mqtt":
        raise cv.Invalid(
            f"Connection type is not 'mqtt' you can not provide '{CONF_MQTT_TOPIC}'."
        )
    return config


INVERTER_CONNECTION_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_TYPE): cv.enum(INVERTER_CONNECTION_TYPE_OPTIONS),
        cv.Optional(CONF_CANBUS_ID): cv.use_id(CanbusComponent),
        cv.Optional(CONF_MQTT_TOPIC): cv.publish_topic,  # only if type is mqtt
    }
).add_extra(validate_inverter_connection)


MASTER_CONNECTION_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_TYPE): cv.enum(MASTER_CONNECTION_TYPE_OPTIONS),
        cv.Optional(CONF_CANBUS_ID): cv.use_id(CanbusComponent),
        cv.Optional(CONF_MASTER_DOMAIN): cv.domain,  # only if type is wifi
    }
).add_extra(validate_master_connection)


BMS_TO_INVERTER_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(CONF_ID): cv.All(
                cv.declare_id(BmsToInverter),
            ),
            cv.Optional(CONF_DEVICE_ROLE, default="master"): cv.enum(
                DEVICE_ROLE_OPTIONS
            ),
            cv.Optional(CONF_INVERTER_TYPE, default="generic"): cv.enum(
                INVERTER_TYPE_OPTIONS
            ),
            cv.Optional(CONF_INVERTER_CONNECTION): INVERTER_CONNECTION_SCHEMA,
            cv.Optional(CONF_MASTER_CONNECTION): MASTER_CONNECTION_SCHEMA,
        }
    )
    .add_extra(validate_BmsToInverter)
    .extend(cv.ENTITY_BASE_SCHEMA)
    .extend(cv.polling_component_schema("5s"))
)
# BMS_TO_INVERTER_SCHEMA = (
#     cv.Schema(
#         {
#             cv.GenerateID(CONF_ID): cv.All(
#                 cv.declare_id(BmsToInverter),
#             ),
#             cv.Optional(CONF_DEVICE_ROLE, default="master"): cv.enum(
#                 DEVICE_ROLE_OPTIONS
#             ),
#             cv.Optional(CONF_INVERTER_TYPE, default="generic"): cv.enum(
#                 INVERTER_TYPE_OPTIONS
#             ),
#         }
#     )
#     .add_extra(validate_BmsToInverter)
#     .extend(cv.ENTITY_BASE_SCHEMA)
#     .extend(cv.polling_component_schema("5s"))
# )


def bms_to_inverter_schema(class_: MockObjClass) -> cv.Schema:
    schema = {cv.GenerateID(): cv.declare_id(class_)}
    return BMS_TO_INVERTER_SCHEMA.extend(schema)


async def setup_bms_to_inverter_core_(bms_to_inverter_var, config):
    cg.add(bms_to_inverter_var.set_device_role(config[CONF_DEVICE_ROLE]))
    if config[CONF_DEVICE_ROLE] == "master":
        config_connection = config[CONF_INVERTER_CONNECTION]
        cg.add(bms_to_inverter_var.set_inverter_type(config[CONF_INVERTER_TYPE]))
        cg.add(
            bms_to_inverter_var.set_inverter_connection_type(
                config_connection[CONF_TYPE]
            )
        )
        if config_connection[CONF_TYPE] == "can":
            canbus_component = await cg.get_variable(config_connection[CONF_CANBUS_ID])
            cg.add(bms_to_inverter_var.set_canbus(canbus_component))
        elif config_connection[CONF_TYPE] == "mqtt":
            if CONF_MQTT_TOPIC not in config_connection:
                config_connection[CONF_MQTT_TOPIC] = DEFAULT_MQTT_TOPIC
            cg.add(bms_to_inverter_var.set_topic(config_connection[CONF_MQTT_TOPIC]))
    elif config[CONF_DEVICE_ROLE] == "slave":
        ...


async def register_bms_to_inverter(var, config):
    if not CORE.has_id(config[CONF_ID]):
        var = cg.Pvariable(config[CONF_ID], var)
    await setup_bms_to_inverter_core_(var, config)


async def new_bms_to_inverter(config, *args):
    var = cg.new_Pvariable(config[CONF_ID], *args)
    await register_bms_to_inverter(var, config)
    return var


async def to_code(config): ...
