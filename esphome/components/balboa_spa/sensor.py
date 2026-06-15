"""
Déclaration des sensors du composant Balboa Spa pour ESPHome.
Ce fichier est lu par le générateur ESPHome côté PC — il ne tourne pas sur l'ESP32.
"""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
)
from . import BalboaSpaComponent, balboa_spa_ns

# ─── Clés YAML pour les deux sensors de température ──────────────────────────
CONF_WATER_TEMPERATURE = "water_temperature"
CONF_SET_TEMPERATURE   = "set_temperature"

# ─── Dépendance au composant parent ──────────────────────────────────────────
DEPENDENCIES = ["balboa_spa"]

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ID): cv.use_id(BalboaSpaComponent),
        cv.Optional(CONF_WATER_TEMPERATURE): sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_SET_TEMPERATURE): sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_ID])

    if CONF_WATER_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_WATER_TEMPERATURE])
        cg.add(parent.set_water_temp_sensor(sens))

    if CONF_SET_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_SET_TEMPERATURE])
        cg.add(parent.set_set_temp_sensor(sens))
