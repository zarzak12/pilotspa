"""
Déclaration des binary sensors du composant Balboa Spa pour ESPHome.
"""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_HEAT,
    DEVICE_CLASS_RUNNING,
    DEVICE_CLASS_LIGHT,
)
from . import BalboaSpaComponent, balboa_spa_ns

CONF_HEATER     = "heater"
CONF_PUMP1      = "pump1"
CONF_PUMP2      = "pump2"
CONF_BLOWER     = "blower"
CONF_LIGHT      = "light"
CONF_FILTRATION = "filtration"
CONF_FILTER1    = "filter1"
CONF_FILTER2    = "filter2"
CONF_AM          = "am"
CONF_PM          = "pm"
CONF_START       = "filter_start"
CONF_STOP        = "filter_stop"
CONF_SET_TIME    = "set_time"
CONF_SET_FILTER  = "set_filter"

DEPENDENCIES = ["balboa_spa"]

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ID): cv.use_id(BalboaSpaComponent),
        cv.Optional(CONF_HEATER):     binary_sensor.binary_sensor_schema(device_class=DEVICE_CLASS_HEAT),
        cv.Optional(CONF_PUMP1):      binary_sensor.binary_sensor_schema(device_class=DEVICE_CLASS_RUNNING),
        cv.Optional(CONF_PUMP2):      binary_sensor.binary_sensor_schema(device_class=DEVICE_CLASS_RUNNING),
        cv.Optional(CONF_BLOWER):     binary_sensor.binary_sensor_schema(),
        cv.Optional(CONF_LIGHT):      binary_sensor.binary_sensor_schema(device_class=DEVICE_CLASS_LIGHT),
        cv.Optional(CONF_FILTRATION): binary_sensor.binary_sensor_schema(device_class=DEVICE_CLASS_RUNNING),
        cv.Optional(CONF_FILTER1):    binary_sensor.binary_sensor_schema(),
        cv.Optional(CONF_FILTER2):    binary_sensor.binary_sensor_schema(),
        cv.Optional(CONF_AM):         binary_sensor.binary_sensor_schema(),
        cv.Optional(CONF_PM):         binary_sensor.binary_sensor_schema(),
        cv.Optional(CONF_START):      binary_sensor.binary_sensor_schema(),
        cv.Optional(CONF_STOP):       binary_sensor.binary_sensor_schema(),
        cv.Optional(CONF_SET_TIME):   binary_sensor.binary_sensor_schema(),
        cv.Optional(CONF_SET_FILTER): binary_sensor.binary_sensor_schema(),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_ID])

    for key, setter in [
        (CONF_HEATER,     "set_heater_sensor"),
        (CONF_PUMP1,      "set_pump1_sensor"),
        (CONF_PUMP2,      "set_pump2_sensor"),
        (CONF_BLOWER,     "set_blower_sensor"),
        (CONF_LIGHT,      "set_light_sensor"),
        (CONF_FILTRATION, "set_filtration_sensor"),
        (CONF_FILTER1,    "set_filter1_sensor"),
        (CONF_FILTER2,    "set_filter2_sensor"),
        
        (CONF_AM,         "set_am_sensor"),
        (CONF_PM,         "set_pm_sensor"),
        (CONF_START,      "set_filter_start_sensor"),
        (CONF_STOP,       "set_filter_stop_sensor"),
        (CONF_SET_TIME,   "set_set_time_sensor"),
        (CONF_SET_FILTER, "set_set_filter_sensor"),

    ]:
        if key in config:
            sens = await binary_sensor.new_binary_sensor(config[key])
            cg.add(getattr(parent, setter)(sens))
