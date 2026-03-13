import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome.components import sensor, text_sensor, binary_sensor 

CODEOWNERS = ["@zarzak12"]

balboa_ns = cg.esphome_ns.namespace("balboa_gs")
BalboaGSComponent = balboa_ns.class_("BalboaGSComponent", cg.Component)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(BalboaGSComponent),

    cv.Optional("temperature_eau"): cv.use_id(sensor.Sensor),
    cv.Optional("temperature_reglage"): cv.use_id(sensor.Sensor),
    
    cv.Optional("affichage"): cv.use_id(text_sensor.TextSensor),
    cv.Optional("mode_affichage"): cv.use_id(text_sensor.TextSensor),
    cv.Optional("heure_spa"): cv.use_id(text_sensor.TextSensor),
    
    cv.Optional("chauffage_active"): cv.use_id(binary_sensor.BinarySensor),
    cv.Optional("pompe1_active"): cv.use_id(binary_sensor.BinarySensor),
    cv.Optional("pompe2_active"): cv.use_id(binary_sensor.BinarySensor),
    cv.Optional("blower_active"): cv.use_id(binary_sensor.BinarySensor),
    cv.Optional("lumiere_active"): cv.use_id(binary_sensor.BinarySensor),
    cv.Optional("filtration_active"): cv.use_id(binary_sensor.BinarySensor),
    cv.Optional("filtre1_active"): cv.use_id(binary_sensor.BinarySensor),
    cv.Optional("filtre2_active"): cv.use_id(binary_sensor.BinarySensor),
    cv.Optional("am_mode"): cv.use_id(binary_sensor.BinarySensor),
    cv.Optional("pm_mode"): cv.use_id(binary_sensor.BinarySensor),
    cv.Optional("start_actif"): cv.use_id(binary_sensor.BinarySensor),
    cv.Optional("stop_actif"): cv.use_id(binary_sensor.BinarySensor),
    cv.Optional("stdMode_actif"): cv.use_id(binary_sensor.BinarySensor),
    cv.Optional("slpMode_actif"): cv.use_id(binary_sensor.BinarySensor),
    cv.Optional("ecnMode_actif"): cv.use_id(binary_sensor.BinarySensor),
    cv.Optional("iceMode_actif"): cv.use_id(binary_sensor.BinarySensor),
}).extend(cv.COMPONENT_SCHEMA)



async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    bindings = [
        ("temperature_eau", "set_temperature_eau_sensor"),
        ("temperature_reglage", "set_temperature_reglage_sensor"),
        ("affichage", "set_affichage_text"),
        ("mode_affichage", "set_mode_affichage_text"),
        ("heure_spa", "set_heure_spa_text"),
        ("chauffage_active", "set_chauffage_binary"),
        ("pompe1_active", "set_pompe1_binary"),
        ("pompe2_active", "set_pompe2_binary"),
        ("blower_active", "set_blower_binary"),
        ("lumiere_active", "set_lumiere_binary"),
        ("filtration_active", "set_filtration_binary"),
        ("filtre1_active", "set_filtre1_binary"),
        ("filtre2_active", "set_filtre2_binary"),
        ("am_mode", "set_am_mode_binary"),
        ("pm_mode", "set_pm_mode_binary"),
        ("start_actif", "set_start_binary"),
        ("stop_actif", "set_stop_binary"),
        ("stdMode_actif", "set_stdMode_binary"),
        ("slpMode_actif", "set_slpMode_binary"),
        ("ecnMode_actif", "set_ecnMode_binary"),
        ("iceMode_actif", "set_iceMode_binary"),
    ]

    for key, setter in bindings:
        if key in config:
            obj = await cg.get_variable(config[key])
            cg.add(getattr(var, setter)(obj))
