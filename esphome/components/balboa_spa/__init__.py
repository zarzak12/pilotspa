"""
Balboa Spa — ESPHome External Component
Adapté depuis Balboa_GS_Interface (MagnusPer)
Compatible ESP32 + ESPHome >= 2024.x
"""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

# ─── Namespace & classe C++ ───────────────────────────────────────────────────
balboa_spa_ns = cg.esphome_ns.namespace("balboa_spa")
BalboaSpaComponent = balboa_spa_ns.class_(
    "BalboaSpaComponent", cg.Component
)

# ─── Clés de configuration YAML ──────────────────────────────────────────────
CONF_CLOCK_PIN  = "clock_pin"
CONF_DATA_PIN   = "data_pin"
CONF_BUTTON_PIN = "button_pin"

# ─── Schéma de validation ─────────────────────────────────────────────────────
# On accepte un entier brut (numéro GPIO) — plus simple et compatible
# avec toutes les versions d'ESPHome sans dépendre de GPIOPin::get_pin()
CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(BalboaSpaComponent),
        cv.Required(CONF_CLOCK_PIN):  cv.positive_int,   # ex: 23
        cv.Required(CONF_DATA_PIN):   cv.positive_int,   # ex: 22
        cv.Required(CONF_BUTTON_PIN): cv.positive_int,   # ex: 13
    }
).extend(cv.COMPONENT_SCHEMA)


# ─── Génération du code C++ ───────────────────────────────────────────────────
async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    # On passe directement les numéros de pin en uint8_t
    cg.add(var.set_clock_pin_num(config[CONF_CLOCK_PIN]))
    cg.add(var.set_data_pin_num(config[CONF_DATA_PIN]))
    cg.add(var.set_button_pin_num(config[CONF_BUTTON_PIN]))
