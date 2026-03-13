#pragma once

#include "esphome.h"
#include "Balboa_GS_Interface.h"

namespace esphome {
namespace balboa_gs {

class BalboaGSComponent : public Component {
 public:

  BalboaGSComponent();
  
  // Capteurs
  sensor::Sensor *temperature_eau_sensor = nullptr;
  sensor::Sensor *temperature_reglage_sensor = nullptr;

  text_sensor::TextSensor *affichage_text = nullptr;
  text_sensor::TextSensor *mode_affichage_text = nullptr;
  text_sensor::TextSensor *heure_spa_text = nullptr;

  // Binary sensors
  binary_sensor::BinarySensor *chauffage_binary = nullptr;
  binary_sensor::BinarySensor *pompe1_binary = nullptr;
  binary_sensor::BinarySensor *pompe2_binary = nullptr;
  binary_sensor::BinarySensor *blower_binary = nullptr;
  binary_sensor::BinarySensor *lumiere_binary = nullptr;
  binary_sensor::BinarySensor *filtration_binary = nullptr;
  binary_sensor::BinarySensor *filtre1_binary = nullptr;
  binary_sensor::BinarySensor *filtre2_binary = nullptr;
  binary_sensor::BinarySensor *am_mode_binary = nullptr;
  binary_sensor::BinarySensor *pm_mode_binary = nullptr;
  binary_sensor::BinarySensor *start_binary = nullptr;
  binary_sensor::BinarySensor *stop_binary = nullptr;
  binary_sensor::BinarySensor *stdMode = nullptr;
  binary_sensor::BinarySensor *slpMode = nullptr;
  binary_sensor::BinarySensor *ecnMode = nullptr;
  binary_sensor::BinarySensor *iceMode = nullptr;
  

  // Interface spa
  BalboaInterface balboa_interface;

  // Déclarations
  void setup() override;
  void loop() override;

  void temp_up();
  void temp_down();
  void pump1_toggle();
  void pump2_toggle();
  void blower_toggle();
  void light_toggle();
  void mode_std();
  void mode_eco();
  void mode_sleep();
  void send_command(const std::string &cmd);
  
  //généré par copilot
  void set_temperature_eau_sensor(sensor::Sensor *s) { temperature_eau_sensor = s; }
  void set_temperature_reglage_sensor(sensor::Sensor *s) { temperature_reglage_sensor = s; }

  void set_affichage_text(text_sensor::TextSensor *s) { affichage_text = s; }
  void set_mode_affichage_text(text_sensor::TextSensor *s) { mode_affichage_text = s; }
  void set_heure_spa_text(text_sensor::TextSensor *s) { heure_spa_text = s; }

  void set_chauffage_binary(binary_sensor::BinarySensor *s) { chauffage_binary = s; }
  void set_pompe1_binary(binary_sensor::BinarySensor *s) { pompe1_binary = s; }
  void set_pompe2_binary(binary_sensor::BinarySensor *s) { pompe2_binary = s; }
  void set_blower_binary(binary_sensor::BinarySensor *s) { blower_binary = s; }
  void set_lumiere_binary(binary_sensor::BinarySensor *s) { lumiere_binary = s; }
  void set_filtration_binary(binary_sensor::BinarySensor *s) { filtration_binary = s; }
  void set_filtre1_binary(binary_sensor::BinarySensor *s) { filtre1_binary = s; }
  void set_filtre2_binary(binary_sensor::BinarySensor *s) { filtre2_binary = s; }
  void set_am_mode_binary(binary_sensor::BinarySensor *s) { am_mode_binary = s; }
  void set_pm_mode_binary(binary_sensor::BinarySensor *s) { pm_mode_binary = s; }
  void set_start_binary(binary_sensor::BinarySensor *s) { start_binary = s; }
  void set_stop_binary(binary_sensor::BinarySensor *s) { stop_binary = s; }
  void set_stdMode_binary(binary_sensor::BinarySensor *s) { stdMode = s; }
  void set_slpMode_binary(binary_sensor::BinarySensor *s) { slpMode = s; }
  void set_ecnMode_binary(binary_sensor::BinarySensor *s) { ecnMode = s; }
  void set_iceMode_binary(binary_sensor::BinarySensor *s) { iceMode = s; }


};

}  // namespace balboa_gs
}  // namespace esphome
