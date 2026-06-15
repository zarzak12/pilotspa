"""
Déclaration des text sensors du composant Balboa Spa pour ESPHome.
"""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import CONF_ID
from . import BalboaSpaComponent, balboa_spa_ns

CONF_LCD_DISPLAY     = "lcd_display"
CONF_MODE            = "mode"
CONF_TIME            = "time"
CONF_FILTER1_START   = "filter1_start"
CONF_FILTER1_STOP    = "filter1_stop"
CONF_FILTER2_START   = "filter2_start"
CONF_FILTER2_STOP    = "filter2_stop"

DEPENDENCIES = ["balboa_spa"]

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ID): cv.use_id(BalboaSpaComponent),
        cv.Optional(CONF_LCD_DISPLAY):   text_sensor.text_sensor_schema(),
        cv.Optional(CONF_MODE):          text_sensor.text_sensor_schema(),
        cv.Optional(CONF_TIME):          text_sensor.text_sensor_schema(),
        cv.Optional(CONF_FILTER1_START): text_sensor.text_sensor_schema(),
        cv.Optional(CONF_FILTER1_STOP):  text_sensor.text_sensor_schema(),
        cv.Optional(CONF_FILTER2_START): text_sensor.text_sensor_schema(),
        cv.Optional(CONF_FILTER2_STOP):  text_sensor.text_sensor_schema(),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_ID])

    if CONF_LCD_DISPLAY in config:
        sens = await text_sensor.new_text_sensor(config[CONF_LCD_DISPLAY])
        cg.add(parent.set_lcd_sensor(sens))

    if CONF_MODE in config:
        sens = await text_sensor.new_text_sensor(config[CONF_MODE])
        cg.add(parent.set_mode_sensor(sens))
    
    if CONF_TIME in config:
        sens = await text_sensor.new_text_sensor(config[CONF_TIME])
        cg.add(parent.set_time_sensor(sens))
        
    if CONF_FILTER1_START in config:
        sens = await text_sensor.new_text_sensor(config[CONF_FILTER1_START])
        cg.add(parent.set_filter1_start_sensor(sens))
    
    if CONF_FILTER1_STOP in config:
        sens = await text_sensor.new_text_sensor(config[CONF_FILTER1_STOP])
        cg.add(parent.set_filter1_stop_sensor(sens))
    
    if CONF_FILTER2_START in config:
        sens = await text_sensor.new_text_sensor(config[CONF_FILTER2_START])
        cg.add(parent.set_filter2_start_sensor(sens))
    
    if CONF_FILTER2_STOP in config:
        sens = await text_sensor.new_text_sensor(config[CONF_FILTER2_STOP])
        cg.add(parent.set_filter2_stop_sensor(sens))

