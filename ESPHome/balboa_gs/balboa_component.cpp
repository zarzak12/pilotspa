#include "balboa_component.h"

namespace esphome {
namespace balboa_gs {

// Définition directe des pins
static const uint8_t CLK_PIN = 23;
static const uint8_t READ_PIN = 22;
static const uint8_t WRITE_PIN = 13;

// Création interface Balboa
BalboaGSComponent::BalboaGSComponent()
  : balboa_interface(CLK_PIN, READ_PIN, WRITE_PIN) {}

// =========================
//        SETUP()
// =========================
void BalboaGSComponent::setup() {
    // Initialisation interface Balboa
    balboa_interface.begin();
}

// =========================
//        LOOP()
// =========================
void BalboaGSComponent::loop() {
    // Mise à jour de l'interface Balboa
    static uint32_t last_run = 0;
    if (millis() - last_run < 20)
        return;

    last_run = millis();

    if (!balboa_interface.loop())
        return;

    // températures filtrées
    float wt = balboa_interface.waterTemperature;
    if (wt > 8 && wt < 42)
        if (temperature_eau_sensor)
        temperature_eau_sensor->publish_state(wt);
    
    // températures filtrées
    float wt_param = balboa_interface.setTemperature;
    if (wt_param > 15 && wt_param < 41)
        if (temperature_reglage_sensor)
        temperature_reglage_sensor->publish_state(wt_param);
    
    //ESP_LOGD("balboa", "LCD_display: %s", balboa_interface.LCD_display.c_str());
    //ESP_LOGD("balboa", "displayMode: %s", balboa_interface.displayMode.c_str());
    

    static std::string last_good_display;
    static std::string last_read_display;
    static unsigned long stable_since_display = 0;
    
    std::string current_display = balboa_interface.LCD_display.c_str();
    
    if (current_display == last_read_display) {
        if (millis() - stable_since_display > 800) { // 300 ms de stabilité
            if (current_display != last_good_display) {
                last_good_display = current_display;
    
                if (affichage_text)
                    affichage_text->publish_state(current_display);
            }
        }
    } else {
        last_read_display = current_display;
        stable_since_display = millis();
    }
    
    //if (affichage_text)
    //    affichage_text->publish_state(std::string(balboa_interface.LCD_display.c_str()));
        
    static std::string last_good_mode;
    static std::string last_read_mode;
    static unsigned long stable_since = 0;
    
    std::string current = balboa_interface.displayMode.c_str();
    
    if (current == last_read_mode) {
        if (millis() - stable_since > 800) { // 300 ms de stabilité
            if (current != last_good_mode) {
                last_good_mode = current;
    
                if (mode_affichage_text)
                    mode_affichage_text->publish_state(current);
            }
        }
    } else {
        last_read_mode = current;
        stable_since = millis();
    }

    //if (mode_affichage_text)
    //    mode_affichage_text->publish_state(std::string(balboa_interface.displayMode.c_str()));
        
        // --- Gestion displayMode comme dans Arduino ---
    if (balboa_interface.displayMode == " Std") {
        if (stdMode) stdMode->publish_state(true);
        if (ecnMode) ecnMode->publish_state(false);
        if (slpMode) slpMode->publish_state(false);
        if (iceMode) iceMode->publish_state(false);
    } else if (balboa_interface.displayMode == " Ecn") {
        if (stdMode) stdMode->publish_state(false);
        if (ecnMode) ecnMode->publish_state(true);
        if (slpMode) slpMode->publish_state(false);
        if (iceMode) iceMode->publish_state(false);
    } else if (balboa_interface.displayMode == " SLP") {
        if (stdMode) stdMode->publish_state(false);
        if (ecnMode) ecnMode->publish_state(false);
        if (slpMode) slpMode->publish_state(true);
        if (iceMode) iceMode->publish_state(false);
    } else if (balboa_interface.displayMode == " 1CE") {
        if (stdMode) stdMode->publish_state(false);
        if (ecnMode) ecnMode->publish_state(false);
        if (slpMode) slpMode->publish_state(false);
        if (iceMode) iceMode->publish_state(true);
    } else {
        if (stdMode) stdMode->publish_state(false);
        if (ecnMode) ecnMode->publish_state(false);
        if (slpMode) slpMode->publish_state(false);
        if (iceMode) iceMode->publish_state(false);
    }

    if (heure_spa_text) {
        char buffer[6];  // format HH:MM
        snprintf(buffer, sizeof(buffer), "%02d:%02d", balboa_interface.heureLue, balboa_interface.minuteLue);
        heure_spa_text->publish_state(std::string(buffer));
    }

    // Binary sensors
    if (chauffage_binary) chauffage_binary->publish_state(balboa_interface.displayHeater);
    if (pompe1_binary) pompe1_binary->publish_state(balboa_interface.displayPump1);
    if (pompe2_binary) pompe2_binary->publish_state(balboa_interface.displayPump2);
    if (blower_binary) blower_binary->publish_state(balboa_interface.displayBlower);
    if (lumiere_binary) lumiere_binary->publish_state(balboa_interface.displayLight);
    if (filtration_binary) filtration_binary->publish_state(balboa_interface.displayFiltration);
    if (am_mode_binary) am_mode_binary->publish_state(balboa_interface.displayAM);
    if (pm_mode_binary) pm_mode_binary->publish_state(balboa_interface.displayPM);
    if (start_binary) start_binary->publish_state(balboa_interface.displayStart);
    if (stop_binary) stop_binary->publish_state(balboa_interface.displayStop);

}

// =========================
//    SEND_COMMAND()
// =========================
void BalboaGSComponent::send_command(const std::string &cmd) {
    // Utiliser les bools statiques de BalboaInterface pour simuler les commandes
    if (cmd == "TEMP_UP"){
        BalboaInterface::writeDisplayData = true;
        BalboaInterface::writeTempUp = true;
    }
    else if (cmd == "TEMP_DOWN"){
        BalboaInterface::writeDisplayData = true;
        BalboaInterface::writeTempDown = true;
    }
    else if (cmd == "PUMP1_TOGGLE"){
        BalboaInterface::writeDisplayData = true;
        BalboaInterface::writePump1 = true;
    }
    else if (cmd == "PUMP2_TOGGLE"){
        BalboaInterface::writeDisplayData = true;
        BalboaInterface::writePump2 = true;
    }
    else if (cmd == "BLOWER_TOGGLE"){
        BalboaInterface::writeDisplayData = true;
        BalboaInterface::writeBlower = true;
    }    
    else if (cmd == "LIGHT_TOGGLE"){
        BalboaInterface::writeDisplayData = true;
        BalboaInterface::writeLight = true;
    // Ajouter d'autres commandes si besoin
    }
}

// =========================
//    COMMANDES DIRECTES
// =========================
void BalboaGSComponent::temp_up() { BalboaInterface::writeDisplayData = true; BalboaInterface::writeTempUp = true; }
void BalboaGSComponent::temp_down() { BalboaInterface::writeDisplayData = true; BalboaInterface::writeTempDown = true; }
void BalboaGSComponent::pump1_toggle() { BalboaInterface::writeDisplayData = true; BalboaInterface::writePump1 = true; }
void BalboaGSComponent::pump2_toggle() { BalboaInterface::writeDisplayData = true; BalboaInterface::writePump2 = true; }
void BalboaGSComponent::blower_toggle() { BalboaInterface::writeDisplayData = true; BalboaInterface::writeBlower = true; }
void BalboaGSComponent::light_toggle() { BalboaInterface::writeDisplayData = true; BalboaInterface::writeLight = true; }
void BalboaGSComponent::mode_std() { BalboaInterface::writeDisplayData = true; BalboaInterface::writeMode = true; }  // À adapter selon la logique de ton interface
void BalboaGSComponent::mode_eco() { BalboaInterface::writeDisplayData = true; BalboaInterface::writeMode = true; }  // Idem
void BalboaGSComponent::mode_sleep() { BalboaInterface::writeDisplayData = true; BalboaInterface::writeMode = true; } // Idem

}  // namespace balboa_gs
}  // namespace esphome
