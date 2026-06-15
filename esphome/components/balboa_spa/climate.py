"""
Déclaration du climate du composant Balboa Spa (proxy bidirectionnel).
Compatible ESPHome >= 2024.x où CLIMATE_SCHEMA est remplacé par climate_schema()
"""
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate
from esphome.const import CONF_ID
from . import BalboaSpaComponent, balboa_spa_ns

CONF_BALBOA_ID = "balboa_id"

DEPENDENCIES = ["balboa_spa"]

BalboaClimate = balboa_spa_ns.class_(
    "BalboaClimate", climate.Climate, cg.Component
)

# climate_schema() est la fonction publique dans ESPHome >= 2024
CONFIG_SCHEMA = climate.climate_schema(BalboaClimate).extend(
    {
        cv.Required(CONF_BALBOA_ID): cv.use_id(BalboaSpaComponent),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await climate.register_climate(var, config)

    parent = await cg.get_variable(config[CONF_BALBOA_ID])
    cg.add(var.set_balboa(parent))
    cg.add(parent.set_climate(var))
