import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor
from esphome.const import (
    CONF_ID,
    CONF_INTEGRATION_TIME,
    CONF_GLASS_ATTENUATION_FACTOR,
    CONF_GAIN,
    CONF_WHITE,
    UNIT_LUX,
    ICON_BRIGHTNESS_6,
    DEVICE_CLASS_ILLUMINANCE,
    STATE_CLASS_MEASUREMENT
)

CONF_AMBIENT_LIGHT = "ambient_light"

CODEOWNERS = ["@latonita"]
DEPENDENCIES = ["i2c"]

veml7700_ns = cg.esphome_ns.namespace("veml7700")

VEML7700Component = veml7700_ns.class_(
    "VEML7700Component", cg.PollingComponent, i2c.I2CDevice
)

VEML7700Gain = veml7700_ns.enum("VEML7700Gain")
GAINS = {
    "X2": VEML7700Gain.VEML7700_GAIN_2,
    "X1": VEML7700Gain.VEML7700_GAIN_1,
    "X1/4": VEML7700Gain.VEML7700_GAIN_1_4,
    "X1/8": VEML7700Gain.VEML7700_GAIN_1_8,
}

VEML7700IntegrationTime = veml7700_ns.enum("VEML7700IntegrationTime")
INTEGRATION_TIMES = {
    25: VEML7700IntegrationTime.VEML7700_INTEGRATION_TIME_25MS,
    50: VEML7700IntegrationTime.VEML7700_INTEGRATION_TIME_50MS,
    100: VEML7700IntegrationTime.VEML7700_INTEGRATION_TIME_100MS,
    200: VEML7700IntegrationTime.VEML7700_INTEGRATION_TIME_200MS,
    400: VEML7700IntegrationTime.VEML7700_INTEGRATION_TIME_400MS,
    800: VEML7700IntegrationTime.VEML7700_INTEGRATION_TIME_800MS,
}

def validate_integration_time(value):
    value = cv.positive_time_period_milliseconds(value).total_milliseconds
    return cv.enum(INTEGRATION_TIMES, int=True)(value)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(VEML7700Component),
            cv.Optional(CONF_AMBIENT_LIGHT): sensor.sensor_schema(
                unit_of_measurement=UNIT_LUX,
                icon=ICON_BRIGHTNESS_6,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_ILLUMINANCE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_WHITE): sensor.sensor_schema(
                unit_of_measurement=UNIT_LUX,
                icon=ICON_BRIGHTNESS_6,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_ILLUMINANCE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_GAIN, default="X1"): cv.enum(GAINS),
            cv.Optional(CONF_INTEGRATION_TIME, default="100ms"): validate_integration_time,
            cv.Optional(CONF_GLASS_ATTENUATION_FACTOR, default=1.0): cv.float_range(min = 1.0),
        }
    )
    .extend(cv.polling_component_schema("60s"))
    .extend(i2c.i2c_device_schema(0x10)),
    cv.has_at_least_one_key(CONF_AMBIENT_LIGHT, CONF_WHITE),
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    if CONF_AMBIENT_LIGHT in config:
        conf = config[CONF_AMBIENT_LIGHT]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_ambient_light_sensor(sens))

    if CONF_WHITE in config:
        conf = config[CONF_WHITE]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_white_sensor(sens))

    cg.add(var.set_gain(config[CONF_GAIN]))
    cg.add(var.set_integration_time(config[CONF_INTEGRATION_TIME]))
    cg.add(var.set_attenuation_factor(config[CONF_GLASS_ATTENUATION_FACTOR]))
