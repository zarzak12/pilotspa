#pragma once

#include "esphome/core/component.h"
#include "esphome/core/application.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/climate/climate.h"

namespace esphome {
namespace balboa_spa {

// ─── Forward declaration ──────────────────────────────────────────────────────
class BalboaClimate;

static const char *const TAG = "balboa_spa";

// ─── Constantes protocole ────────────────────────────────────────────────────
static const uint8_t  BALBOA_BUFFER_SIZE   = 74;
static const uint8_t  BALBOA_DISPLAY_BITS  = 71;
static const uint8_t  BALBOA_TOTAL_BITS    = 75;
static const uint32_t BALBOA_NEW_CYCLE_US  = 5000;
static const uint32_t BALBOA_BTN_INTERVAL  = 500;

// ─── Variables ISR ───────────────────────────────────────────────────────────
static volatile uint8_t  isr_buffer[BALBOA_BUFFER_SIZE];
static volatile bool     isr_buffer_ready    = false;
static volatile bool     isr_buffer_overflow = false;
static volatile uint8_t  isr_data_index      = 0;
static volatile int      isr_bit_counter     = 0;
static volatile uint32_t isr_last_time_us    = 0;

static volatile bool isr_write_pending  = false;
static volatile bool isr_write_mode     = false;
static volatile bool isr_write_temp_up  = false;
static volatile bool isr_write_temp_dn  = false;
static volatile bool isr_write_light    = false;
static volatile bool isr_write_pump1    = false;
static volatile bool isr_write_pump2    = false;
static volatile bool isr_write_blower   = false;
static volatile bool isr_write_time     = false;

static uint8_t isr_clock_pin  = 0;
static uint8_t isr_data_pin   = 0;
static uint8_t isr_button_pin = 0;

static bool callTimeFilter = false;
static bool callTime = false;
static bool traitementEnCours = false;
static bool callSetTime = false;

static int callTimeButtonPresses = 0;
static int callModeButtonPresses = 0;
static int updateTimeFTButtonPresses = 0;
static int updateModeFTButtonPresses = 0;

// ─── ISR ─────────────────────────────────────────────────────────────────────
static void IRAM_ATTR balboa_clock_isr() {
  if (isr_buffer_ready) return;

  uint32_t now = micros();
  if ((now - isr_last_time_us) >= BALBOA_NEW_CYCLE_US) {
    isr_data_index  = 0;
    isr_bit_counter = 0;
  }
  isr_last_time_us = now;

  if (digitalRead(isr_clock_pin) == LOW) {
    digitalWrite(isr_button_pin, LOW);
    return;
  }

  //  Bouton    | 72 | 73 | 74 | 75
  //  Mode      |  1 |  0 |  0 |  0
  //  Temp up   |  1 |  1 |  1 |  0
  //  Temp down |  1 |  1 |  1 |  1
  //  Light     |  1 |  0 |  1 |  1
  //  Pump 1    |  1 |  0 |  0 |  1
  //  Pump 2    |  1 |  0 |  1 |  0
  //  Time      |  1 |  1 |  0 |  0
  //  Blower    |  1 |  1 |  0 |  1
  if (isr_write_pending && isr_bit_counter >= 72 && isr_bit_counter <= 75) {

    if (isr_bit_counter == 72) {
      digitalWrite(isr_button_pin, HIGH);

    } else if (isr_bit_counter == 73) {
      if      (isr_write_mode)    digitalWrite(isr_button_pin, LOW);
      else if (isr_write_temp_up) digitalWrite(isr_button_pin, HIGH);
      else if (isr_write_temp_dn) digitalWrite(isr_button_pin, HIGH);
      else if (isr_write_light)   digitalWrite(isr_button_pin, LOW);
      else if (isr_write_pump1)   digitalWrite(isr_button_pin, LOW);
      else if (isr_write_pump2)   digitalWrite(isr_button_pin, LOW);
      else if (isr_write_time)    digitalWrite(isr_button_pin, HIGH);
      else if (isr_write_blower)  digitalWrite(isr_button_pin, HIGH);

    } else if (isr_bit_counter == 74) {
      if      (isr_write_mode)    digitalWrite(isr_button_pin, LOW);
      else if (isr_write_temp_up) digitalWrite(isr_button_pin, HIGH);
      else if (isr_write_temp_dn) digitalWrite(isr_button_pin, HIGH);
      else if (isr_write_light)   digitalWrite(isr_button_pin, HIGH);
      else if (isr_write_pump1)   digitalWrite(isr_button_pin, LOW);
      else if (isr_write_pump2)   digitalWrite(isr_button_pin, HIGH);
      else if (isr_write_time)    digitalWrite(isr_button_pin, LOW);
      else if (isr_write_blower)  digitalWrite(isr_button_pin, LOW);

    } else if (isr_bit_counter == 75) {
      if      (isr_write_mode)    digitalWrite(isr_button_pin, LOW);
      else if (isr_write_temp_up) digitalWrite(isr_button_pin, LOW);
      else if (isr_write_temp_dn) digitalWrite(isr_button_pin, HIGH);
      else if (isr_write_light)   digitalWrite(isr_button_pin, HIGH);
      else if (isr_write_pump1)   digitalWrite(isr_button_pin, HIGH);
      else if (isr_write_pump2)   digitalWrite(isr_button_pin, LOW);
      else if (isr_write_time)    digitalWrite(isr_button_pin, LOW);
      else if (isr_write_blower)  digitalWrite(isr_button_pin, HIGH);

      isr_write_pending = false;
      isr_write_mode    = false;
      isr_write_temp_up = false;
      isr_write_temp_dn = false;
      isr_write_light   = false;
      isr_write_pump1   = false;
      isr_write_pump2   = false;
      isr_write_time    = false;
      isr_write_blower  = false;
    }
  }

  if (isr_bit_counter <= BALBOA_DISPLAY_BITS) {
    isr_buffer[isr_data_index] = digitalRead(isr_data_pin);
    isr_data_index++;
  } else if (isr_bit_counter == BALBOA_TOTAL_BITS) {
    isr_buffer_ready = true;
    detachInterrupt(digitalPinToInterrupt(isr_clock_pin));
  } else if (isr_bit_counter > BALBOA_TOTAL_BITS) {
    isr_buffer_overflow = true;
  }

  isr_bit_counter++;
}

// ─── Lookup 7-segments ───────────────────────────────────────────────────────
static std::string lcd_char(int seg) {
  switch (seg) {
    case 0b0000000: return " ";
    case 0b1111110: return "0";
    case 0b0110000: return "1";
    case 0b1101101: return "2";
    case 0b1111001: return "3";
    case 0b0110011: return "4";
    case 0b1011011: return "5";
    case 0b1011111: return "6";
    case 0b1110000: return "7";
    case 0b1111111: return "8";
    case 0b1110011: return "9";
    case 0b1110111: return "A";
    case 0b1001110: return "C";
    case 0b1001111: return "E";
    case 0b1011110: return "G";
    case 0b0110111: return "H";
    case 0b0111100: return "J";
    case 0b0001110: return "L";
    case 0b1010100: return "M";
    case 0b1110110: return "N";
    case 0b1100111: return "P";
    case 0b1101011: return "Q";
    case 0b1100110: return "R";
    case 0b0111110: return "U";
    case 0b0101010: return "W";
    case 0b1111101: return "a";
    case 0b0011111: return "b";
    case 0b0001101: return "c";
    case 0b0111101: return "d";
    case 0b1101111: return "e";
    case 0b1000111: return "f";
    case 0b0010111: return "h";
    case 0b0000100: return "i";
    case 0b0000001: return "j";
    case 0b1010111: return "k";
    case 0b0000110: return "l";
    case 0b0010100: return "m";
    case 0b0010101: return "n";
    case 0b0011101: return "o";
    case 0b0000101: return "r";
    case 0b0001111: return "t";
    case 0b0011100: return "u";
    case 0b0111011: return "y";
    default:        return "-";
  }
}

// ═════════════════════════════════════════════════════════════════════════════
//  Classe principale
// ═════════════════════════════════════════════════════════════════════════════
class BalboaSpaComponent : public Component {
 public:

  void set_clock_pin_num(uint8_t pin)  { isr_clock_pin  = pin; }
  void set_data_pin_num(uint8_t pin)   { isr_data_pin   = pin; }
  void set_button_pin_num(uint8_t pin) { isr_button_pin = pin; }

  void set_climate(BalboaClimate *c) { climate_ = c; }

  void set_water_temp_sensor(sensor::Sensor *s)              { water_temp_sensor_   = s; }
  void set_set_temp_sensor(sensor::Sensor *s)                { set_temp_sensor_     = s; }
  void set_lcd_sensor(text_sensor::TextSensor *s)            { lcd_sensor_          = s; }
  void set_mode_sensor(text_sensor::TextSensor *s)           { mode_sensor_         = s; }
  void set_time_sensor(text_sensor::TextSensor *s)           { time_sensor_         = s; }
  void set_heater_sensor(binary_sensor::BinarySensor *s)     { heater_sensor_       = s; }
  void set_pump1_sensor(binary_sensor::BinarySensor *s)      { pump1_sensor_        = s; }
  void set_pump2_sensor(binary_sensor::BinarySensor *s)      { pump2_sensor_        = s; }
  void set_blower_sensor(binary_sensor::BinarySensor *s)     { blower_sensor_       = s; }
  void set_light_sensor(binary_sensor::BinarySensor *s)      { light_sensor_        = s; }
  void set_filtration_sensor(binary_sensor::BinarySensor *s) { filtration_sensor_   = s; }
  void set_filter1_sensor(binary_sensor::BinarySensor *s)    { filter1_sensor_      = s; }
  void set_filter2_sensor(binary_sensor::BinarySensor *s)    { filter2_sensor_      = s; }
  void set_am_sensor(binary_sensor::BinarySensor *s)         { am_sensor_           = s; }
  void set_pm_sensor(binary_sensor::BinarySensor *s)         { pm_sensor_           = s; }
  void set_filter_start_sensor(binary_sensor::BinarySensor *s) { filter_start_      = s; }
  void set_filter_stop_sensor(binary_sensor::BinarySensor *s)  { filter_stop_       = s; }
  void set_set_time_sensor(binary_sensor::BinarySensor *s)   { set_time_sensor_     = s; }
  void set_set_filter_sensor(binary_sensor::BinarySensor *s) { set_filter_sensor_   = s; }

  void set_filter1_start_sensor(text_sensor::TextSensor *s) { filter1_start_sensor_ = s; }
  void set_filter1_stop_sensor(text_sensor::TextSensor *s)  { filter1_stop_sensor_  = s; }
  void set_filter2_start_sensor(text_sensor::TextSensor *s) { filter2_start_sensor_ = s; }
  void set_filter2_stop_sensor(text_sensor::TextSensor *s)  { filter2_stop_sensor_  = s; }

  // ── setup() ──────────────────────────────────────────────────────────────
  void setup() override {
    pinMode(isr_clock_pin,  INPUT);
    pinMode(isr_data_pin,   INPUT);
    pinMode(isr_button_pin, OUTPUT);
    digitalWrite(isr_button_pin, LOW);
    attachInterrupt(digitalPinToInterrupt(isr_clock_pin), balboa_clock_isr, CHANGE);
    ESP_LOGI(TAG, "Initialisé — clock=%d data=%d button=%d",
             isr_clock_pin, isr_data_pin, isr_button_pin);
  }

  float get_setup_priority() const override { return setup_priority::DATA; }

  // ── loop() ───────────────────────────────────────────────────────────────
  void loop() override {
    if (isr_buffer_ready) {
      decode_display_();
      attachInterrupt(digitalPinToInterrupt(isr_clock_pin), balboa_clock_isr, CHANGE);
    }

    if (spa_ready_ && pending_target_ > 0.0f) {
      ESP_LOGI(TAG, "Spa prêt — application de la consigne en attente : %.1f°C", pending_target_);
      apply_target_temperature_(pending_target_);
      pending_target_ = 0.0f;
    }

    if (update_temp_presses_ > 0) {
      uint32_t now = millis();
      if ((now - btn_timer_prev_ms_) > BALBOA_BTN_INTERVAL) {
        btn_timer_prev_ms_ = now;
        isr_write_pending = true;
        if (update_temp_dir_ == 1) isr_write_temp_dn = true;
        else if (update_temp_dir_ == 2) isr_write_temp_up = true;
        update_temp_presses_--;
      }
    }

    // Get Time
    if (callTime) {
      if (millis() - btn_timer_prev_ms_ > BALBOA_BTN_INTERVAL) {
        btn_timer_prev_ms_ = millis();
        if (callTimeButtonPresses > 0) {
          isr_write_pending = true;
          isr_write_time = true;
          callTimeButtonPresses--;
        } else {
          callTimeButtonPresses = 0;
          callTime = false;
          traitementEnCours = false;
        }
      }
    }

    if (callSetTime) {
      if (millis() - btn_timer_prev_ms_ > BALBOA_BTN_INTERVAL) {
        btn_timer_prev_ms_ = millis();
        if (callTimeButtonPresses > 0) {
          isr_write_pending = true;
          isr_write_time = true;
          callTimeButtonPresses--;
        } else if (callModeButtonPresses > 0) {
          isr_write_pending = true;
          isr_write_mode = true;
          callModeButtonPresses--;
        } else {
          callTimeButtonPresses = 0;
          callModeButtonPresses = 0;
          callSetTime = false;
          traitementEnCours = false;
        }
      }
    }

    // Obtention du Filtre de l'Heure
    if (callTimeFilter) {
      if (millis() - btn_timer_prev_ms_ > BALBOA_BTN_INTERVAL) {
        btn_timer_prev_ms_ = millis();
        if (updateTimeFTButtonPresses > 1) {
          isr_write_pending = true;
          isr_write_time = true;
          updateTimeFTButtonPresses--;
        } else if (updateTimeFTButtonPresses == 1) {
          if (updateModeFTButtonPresses > 0) {
            isr_write_pending = true;
            isr_write_mode = true;
            updateModeFTButtonPresses--;
          } else if (updateModeFTButtonPresses == 0) {
            isr_write_pending = true;
            isr_write_time = true;
            updateTimeFTButtonPresses--;
            callTimeFilter = false;
            traitementEnCours = false;
          }
        }
      }
    }

    process_time_sync_();
    process_filter_read_();

    // Ne jamais utiliser delay() dans le loop ESPHome — ça bloque le scheduler
    // FreeRTOS et empêche l'OTA/API de traiter leurs connexions après quelques minutes.
    // feed_wdt() nourrit le watchdog sans bloquer.
    App.feed_wdt();
  }

  void process_filter_read_() {
    if (!filter_read_active_) return;

    if (millis() - btn_timer_prev_ms_ < BALBOA_BTN_INTERVAL) return;
    btn_timer_prev_ms_ = millis();

    switch (filter_state_) {

        // ── TIME → afficher heure ───────────────────────────────
        case FILTER_SHOW_TIME:
        isr_write_pending = true;
        isr_write_time    = true;
        filter_state_     = FILTER_WAIT_TIME;
        filter_read_attempts_ = 0;
        ESP_LOGD(TAG, "FilterRead → TIME (affiche heure)");
        break;

        case FILTER_WAIT_TIME:
        if (last_set_time_ == false && last_set_filter_ == false && last_time_ != "") {
            filter_state_ = FILTER_SET_TIME;
        } else if (++filter_read_attempts_ > 10) {
            ESP_LOGW(TAG, "FilterRead → timeout heure");
            
            ESP_LOGW(TAG, "FilterRead → ABORT");

            filter_read_active_ = false;
            traitementEnCours   = false;
            filter_state_       = FILTER_IDLE;

        }
        break;

        // ── MODE → entrer Set Time ─────────────────────────────
        case FILTER_SET_TIME:
        isr_write_pending = true;
        isr_write_mode    = true;
        filter_state_     = FILTER_WAIT_SET_TIME;
        ESP_LOGD(TAG, "FilterRead → MODE (set time)");
        break;

        case FILTER_WAIT_SET_TIME:
        if (last_set_time_) {
            filter_state_ = FILTER_F1_START;
        }
        break;

        // ── F1 START ───────────────────────────────────────────
        case FILTER_F1_START:
        isr_write_pending = true;
        isr_write_mode    = true;
        filter_state_     = FILTER_WAIT_F1_START;
        ESP_LOGD(TAG, "FilterRead → MODE (F1 start)");
        break;

        case FILTER_WAIT_F1_START:
        if (last_filter1_ && last_filter_start_) {
            filter1_start_ = last_time_;
            if (filter1_start_sensor_) filter1_start_sensor_->publish_state(filter1_start_);
            filter_state_ = FILTER_F1_STOP;
        }
        break;

        // ── F1 STOP ────────────────────────────────────────────
        case FILTER_F1_STOP:
        isr_write_pending = true;
        isr_write_mode    = true;
        filter_state_     = FILTER_WAIT_F1_STOP;
        ESP_LOGD(TAG, "FilterRead → MODE (F1 stop)");
        break;

        case FILTER_WAIT_F1_STOP:
        if (last_filter1_ && last_filter_stop_) {
            filter1_stop_ = last_time_;
            if (filter1_stop_sensor_)  filter1_stop_sensor_->publish_state(filter1_stop_);
            filter_state_ = FILTER_F2_START;
        }
        break;

        // ── F2 START ───────────────────────────────────────────
        case FILTER_F2_START:
        isr_write_pending = true;
        isr_write_mode    = true;
        filter_state_     = FILTER_WAIT_F2_START;
        break;

        case FILTER_WAIT_F2_START:
        if (last_filter2_ && last_filter_start_) {
            filter2_start_ = last_time_;
            if (filter2_start_sensor_) filter2_start_sensor_->publish_state(filter2_start_);
            filter_state_ = FILTER_F2_STOP;
        }
        break;

        // ── F2 STOP ────────────────────────────────────────────
        case FILTER_F2_STOP:
        isr_write_pending = true;
        isr_write_mode    = true;
        filter_state_     = FILTER_WAIT_F2_STOP;
        break;

        case FILTER_WAIT_F2_STOP:
        if (last_filter2_ && last_filter_stop_) {
            filter2_stop_ = last_time_;
            if (filter2_stop_sensor_)  filter2_stop_sensor_->publish_state(filter2_stop_);
            filter_state_ = FILTER_EXIT;
        }
        break;

        // ── SORTIE ─────────────────────────────────────────────
        case FILTER_EXIT:
        isr_write_pending = true;
        isr_write_time    = true;        

        filter_read_active_ = false;
        traitementEnCours   = false;
        filter_state_       = FILTER_IDLE;

        ESP_LOGI(TAG, "FilterRead → terminé");
        break;

        default:
        
        ESP_LOGW(TAG, "FilterRead → ABORT");

        filter_read_active_ = false;
        traitementEnCours   = false;
        filter_state_       = FILTER_IDLE;

        break;
    }
  }
  // ── Séquence complète : lit l'heure spa puis synchronise ─────────────────
  void process_time_sync_() {
    if (!sync_time_active_) return;

    if (millis() - btn_timer_prev_ms_ < BALBOA_BTN_INTERVAL) return;
    btn_timer_prev_ms_ = millis();

    switch (time_sync_state_) {

      // ── Étape 1 : Time x1 → affiche l'heure courante sur le LCD ─────────
      case TIME_SYNC_READ_TIME:
        spa_hour_   = -1;  // invalide : on attend une vraie lecture
        spa_minute_ = -1;
        time_sync_read_attempts_ = 0;
        isr_write_pending = true;
        isr_write_time    = true;
        time_sync_state_  = TIME_SYNC_WAIT_READ;
        ESP_LOGD(TAG, "TimeSync → TIME (lecture heure spa)");
        break;

      // ── Étape 2 : attendre que decode_display_ capture l'heure ──────────
      // spa_hour_ et spa_minute_ sont mis à jour dans decode_display_()
      // dès que b_time=true et que les chiffres sont valides
      case TIME_SYNC_WAIT_READ:
        time_sync_read_attempts_++;
        if (spa_hour_ >= 0 && spa_minute_ >= 0) {
          // Heure capturée — refermer l'affichage
          ESP_LOGI(TAG, "TimeSync → heure spa lue : %02d:%02d", spa_hour_, spa_minute_);
          time_sync_state_ = TIME_SYNC_CLOSE_READ;
        } else if (time_sync_read_attempts_ > 10) {
          // Timeout (~5s) : abandon
          ESP_LOGW(TAG, "TimeSync → timeout lecture heure, abandon");
          sync_time_active_ = false;
          time_sync_state_  = TIME_SYNC_IDLE;
          traitementEnCours = false;
        }
        // sinon on patiente (le prochain passage dans loop() après 500ms)
        break;

      // ── Étape 3 : Time x1 → referme l'affichage heure ───────────────────
      case TIME_SYNC_CLOSE_READ:
        isr_write_pending = true;
        isr_write_time    = true;
        time_sync_state_  = TIME_SYNC_COMPUTE;
        ESP_LOGD(TAG, "TimeSync → TIME (ferme affichage heure)");
        break;

      // ── Étape 4 : calculer le diff ───────────────────────────────────────
      case TIME_SYNC_COMPUTE: {
        // Heure ESP32 via NTP
        auto now_t = ::time(nullptr);
        struct tm tm_now;
        localtime_r(&now_t, &tm_now);
        int esp_minutes = tm_now.tm_hour * 60 + tm_now.tm_min;

        // Heure spa : conversion 12h→24h si AM/PM actif
        int spa_hour_24;

        // Format 12h
        spa_hour_24 = spa_is_pm_
        ? ((spa_hour_ == 12) ? 12 : spa_hour_ + 12)
        : ((spa_hour_ == 12) ? 0  : spa_hour_);

        int spa_minutes = spa_hour_24 * 60 + spa_minute_;

        time_diff_minutes_ = esp_minutes - spa_minutes;

        ESP_LOGW(TAG, "TimeSync: ESP=%02d:%02d (%d min), SPA=%02d:%02d (%d min), diff=%d min",
                 tm_now.tm_hour, tm_now.tm_min, esp_minutes,
                 spa_hour_24, spa_minute_, spa_minutes,
                 time_diff_minutes_);

        if (time_diff_minutes_ == 0) {
          ESP_LOGI(TAG, "TimeSync → spa déjà à l'heure");
          sync_time_active_ = false;
          time_sync_state_  = TIME_SYNC_IDLE;
          traitementEnCours = false;
          break;
        }

        if (abs(time_diff_minutes_) > 12 * 60) {
          ESP_LOGW(TAG, "TimeSync → diff trop grande (%d min), abandon", time_diff_minutes_);
          sync_time_active_ = false;
          time_sync_state_  = TIME_SYNC_IDLE;
          traitementEnCours = false;
          break;
        }

        time_press_dir_   = (time_diff_minutes_ > 0) ? 2 : 1;
        time_press_count_ = abs(time_diff_minutes_) * 2;  // 2 appuis = 1 minute

        ESP_LOGI(TAG, "TimeSync → %d appuis %s",
                 time_press_count_,
                 time_press_dir_ == 2 ? "UP" : "DOWN");

        time_sync_state_ = TIME_SYNC_ENTER;
        break;
      }

      // ── Étape 5 : Time → entre dans le menu set time ─────────────────────
      case TIME_SYNC_ENTER:
        isr_write_pending = true;
        isr_write_time    = true;
        time_sync_state_  = TIME_SYNC_SETMODE;
        ESP_LOGD(TAG, "TimeSync → TIME (entre menu set)");
        break;

      // ── Étape 6 : Mode → passe en mode réglage heure ────────────────────
      case TIME_SYNC_SETMODE:
        isr_write_pending = true;
        isr_write_mode    = true;
        time_sync_state_  = TIME_SYNC_ADJUST;
        ESP_LOGD(TAG, "TimeSync → MODE (set time)");
        break;

      // ── Étape 7 : TempUp / TempDown x N (2 appuis = 1 minute) ───────────
      case TIME_SYNC_ADJUST:
        if (time_press_count_ > 0) {
          isr_write_pending = true;
          if (time_press_dir_ == 2) isr_write_temp_up = true;
          else                      isr_write_temp_dn = true;
          time_press_count_--;
          ESP_LOGD(TAG, "TimeSync → ADJUST reste %d", time_press_count_);
        } else {
          time_sync_state_ = TIME_SYNC_EXIT;
        }
        break;

      // ── Étape 8 : Time → sortie ──────────────────────────────────────────
      case TIME_SYNC_EXIT:
        isr_write_pending = true;
        isr_write_time    = true;
        sync_time_active_ = false;
        time_sync_state_  = TIME_SYNC_IDLE;
        traitementEnCours = false;
        ESP_LOGI(TAG, "TimeSync → terminée");
        break;

      default:
        sync_time_active_ = false;
        time_sync_state_  = TIME_SYNC_IDLE;
        break;
    }
  }

  // ── Commandes publiques ───────────────────────────────────────────────────
  bool can_press_button_() {
    uint32_t now = millis();
    if (now - btn_timer_prev_ms_ < BALBOA_BTN_INTERVAL) return false;
    btn_timer_prev_ms_ = now;
    return true;
  }

  void press_temp_up()   { if (!can_press_button_()) return; isr_write_pending = true; isr_write_temp_up = true; }
  void press_temp_down() { if (!can_press_button_()) return; isr_write_pending = true; isr_write_temp_dn = true; }
  void press_light()     { if (!can_press_button_()) return; isr_write_pending = true; isr_write_light   = true; }
  void press_pump1()     { if (!can_press_button_()) return; isr_write_pending = true; isr_write_pump1   = true; }
  void press_pump2()     { if (!can_press_button_()) return; isr_write_pending = true; isr_write_pump2   = true; }
  void press_blower()    { if (!can_press_button_()) return; isr_write_pending = true; isr_write_blower  = true; }
  void press_mode()      { if (!can_press_button_()) return; isr_write_pending = true; isr_write_mode    = true; }
  void press_time()      { if (!can_press_button_()) return; callTime = true; callTimeButtonPresses = 1; }
  void press_set_times() { if (!can_press_button_()) return; callSetTime = true; callTimeButtonPresses = 1; callModeButtonPresses = 1; }

  void read_filter_times() {
    /*callTimeFilter = true;
    updateTimeFTButtonPresses = 2;
    updateModeFTButtonPresses = 5;
    traitementEnCours = true;*/
    
    if (filter_read_active_) return;

    ESP_LOGI(TAG, "Lecture heures de filtration → démarrage");
    traitementEnCours     = true;
    filter_read_active_   = true;
    filter_state_         = FILTER_SHOW_TIME;
    filter_read_attempts_ = 0;
    btn_timer_prev_ms_    = millis();

  }

  // Démarre la séquence complète : lit l'heure spa puis synchronise avec NTP
  void sync_time() {
    if (sync_time_active_) return;
    ESP_LOGI(TAG, "sync_time → démarrage séquence lecture + sync");
    traitementEnCours  = true;
    sync_time_active_  = true;
    time_sync_state_   = TIME_SYNC_READ_TIME;
    btn_timer_prev_ms_ = millis();
  }

  void set_target_temperature(float target) {
    if (!spa_ready_) {
      pending_target_ = target;
      ESP_LOGI(TAG, "Spa pas encore prêt, consigne HA mémorisée : %.1f°C", target);
      return;
    }
    apply_target_temperature_(target);
  }

  float get_water_temperature() const { return water_temperature_; }
  float get_set_temperature()   const { return set_temperature_;   }
  int   get_pending_presses()   const { return update_temp_presses_; }

 protected:

  BalboaClimate *climate_ = nullptr;

  sensor::Sensor          *water_temp_sensor_   = nullptr;
  sensor::Sensor          *set_temp_sensor_     = nullptr;
  text_sensor::TextSensor *lcd_sensor_          = nullptr;
  text_sensor::TextSensor *mode_sensor_         = nullptr;
  text_sensor::TextSensor *time_sensor_         = nullptr;
  binary_sensor::BinarySensor *heater_sensor_     = nullptr;
  binary_sensor::BinarySensor *pump1_sensor_      = nullptr;
  binary_sensor::BinarySensor *pump2_sensor_      = nullptr;
  binary_sensor::BinarySensor *blower_sensor_     = nullptr;
  binary_sensor::BinarySensor *light_sensor_      = nullptr;
  binary_sensor::BinarySensor *filtration_sensor_ = nullptr;
  binary_sensor::BinarySensor *filter1_sensor_    = nullptr;
  binary_sensor::BinarySensor *filter2_sensor_    = nullptr;
  binary_sensor::BinarySensor *am_sensor_         = nullptr;
  binary_sensor::BinarySensor *pm_sensor_         = nullptr;
  binary_sensor::BinarySensor *filter_start_      = nullptr;
  binary_sensor::BinarySensor *filter_stop_       = nullptr;
  binary_sensor::BinarySensor *set_time_sensor_   = nullptr;
  binary_sensor::BinarySensor *set_filter_sensor_ = nullptr;

  float       water_temperature_ = 0.0f;
  float       set_temperature_   = 0.0f;
  std::string display_mode_      = "Unknown";

  float last_climate_water_ = 0.0f;
  float last_climate_set_   = 0.0f;
  std::string last_climate_mode_ = "";

  int      update_temp_dir_     = 0;
  int      update_temp_presses_ = 0;
  uint32_t btn_timer_prev_ms_   = 0;

  bool  spa_ready_      = false;
  float pending_target_ = 0.0f;

  // ── Throttling ────────────────────────────────────────────────────────────
  static const uint32_t PUBLISH_INTERVAL_MS = 500;
  uint32_t    last_publish_ms_    = 0;
  float       last_water_temp_    = -999.0f;
  float       last_set_temp_      = -999.0f;
  std::string last_lcd_           = "";
  std::string last_mode_          = "";
  std::string last_time_          = "";
  int last_seg1_ = 0, last_seg2_ = 0, last_seg3_ = 0, last_seg4_ = 0;
  bool last_heater_       = false;
  bool last_pump1_        = false;
  bool last_pump2_        = false;
  bool last_blower_       = false;
  bool last_light_        = false;
  bool last_filtration_   = false;
  bool last_filter1_      = false;
  bool last_filter2_      = false;
  bool last_am_           = false;
  bool last_pm_           = false;
  bool last_filter_start_ = false;
  bool last_filter_stop_  = false;
  bool last_set_time_     = false;
  bool last_set_filter_   = false;

  // ── Text sensors heures filtration ───────────────────────────────────────
  text_sensor::TextSensor *filter1_start_sensor_ = nullptr;
  text_sensor::TextSensor *filter1_stop_sensor_  = nullptr;
  text_sensor::TextSensor *filter2_start_sensor_ = nullptr;
  text_sensor::TextSensor *filter2_stop_sensor_  = nullptr;
  
	// ── Lecture heures de filtration ───────────────────────────────────────────
	bool filter_read_active_ = false;

    uint8_t filter_read_attempts_ = 0;

    // buffers temporaires
    std::string filter1_start_;
    std::string filter1_stop_;
    std::string filter2_start_;
    std::string filter2_stop_;

	enum FilterReadState {
    FILTER_IDLE,

    FILTER_SHOW_TIME,
    FILTER_WAIT_TIME,

    FILTER_SET_TIME,
    FILTER_WAIT_SET_TIME,

    FILTER_F1_START,
    FILTER_WAIT_F1_START,

    FILTER_F1_STOP,
    FILTER_WAIT_F1_STOP,

    FILTER_F2_START,
    FILTER_WAIT_F2_START,

    FILTER_F2_STOP,
    FILTER_WAIT_F2_STOP,

    FILTER_EXIT
    };
    FilterReadState filter_state_ = FILTER_IDLE;


  // ── Mise à l'heure spa ────────────────────────────────────────────────────
  bool sync_time_active_        = false;
  int  spa_hour_                = -1;  // -1 = pas encore lu
  int  spa_minute_              = -1;
  int  time_diff_minutes_       = 0;
  int  time_press_count_        = 0;
  int  time_press_dir_          = 0;   // 1=down, 2=up
  uint8_t time_sync_read_attempts_ = 0;
  bool spa_is_pm_ = false;

  enum TimeSyncState {
    TIME_SYNC_IDLE,
    TIME_SYNC_READ_TIME,   // Time x1 → affiche l'heure courante
    TIME_SYNC_WAIT_READ,   // attend la capture de spa_hour_ / spa_minute_
    TIME_SYNC_CLOSE_READ,  // Time x1 → referme l'affichage heure
    TIME_SYNC_COMPUTE,     // calcule le diff et prépare les appuis
    TIME_SYNC_ENTER,       // Time x1 → entre dans le menu set time
    TIME_SYNC_SETMODE,     // Mode x1 → passe en mode réglage
    TIME_SYNC_ADJUST,      // TempUp / TempDown x N (2 appuis = 1 minute)
    TIME_SYNC_EXIT         // Time x1 → sortie
  };
  TimeSyncState time_sync_state_ = TIME_SYNC_IDLE;

  // ── Décodage buffer ───────────────────────────────────────────────────────
  void decode_display_() {
    uint8_t buf[BALBOA_BUFFER_SIZE];
    {
      InterruptLock lock;
      memcpy(buf, (const void *) isr_buffer, BALBOA_BUFFER_SIZE);
      isr_buffer_ready = false;
    }

    int  seg1 = 0, seg2 = 0, seg3 = 0, seg4 = 0;
    bool bit0 = (buf[0] == 1);

    bool b_button = false, b_blower = false, b_light = false;
    bool b_pump1  = false, b_pump2  = false;
    bool b_filter1 = false, b_filter2 = false, b_filtration = false;
    bool b_heater  = false, b_std = false, b_eco = false;
    bool b_am = false, b_pm = false, b_time = false;
    bool b_start = false, b_stop = false;
    bool b_set_time = false, b_set_filter = false;
    bool b_set_heat = false;

    last_seg1_ = 0; last_seg2_ = 0; last_seg3_ = 0; last_seg4_ = 0;

    for (int x = 1; x <= (int)BALBOA_DISPLAY_BITS; x++) {
      bool bit = (buf[x] == 1);

      if      (x >= 1  && x <= 7)  { bit0 ? (seg1 = (seg1<<1)|(bit?1:0)) : (seg4 = (seg4<<1)|(bit?1:0)); }
      else if (x >= 8  && x <= 14) { bit0 ? (seg2 = (seg2<<1)|(bit?1:0)) : (seg3 = (seg3<<1)|(bit?1:0)); }
      else if (x >= 15 && x <= 21) { bit0 ? (seg3 = (seg3<<1)|(bit?1:0)) : (seg2 = (seg2<<1)|(bit?1:0)); }
      else if (x >= 22 && x <= 28) { bit0 ? (seg4 = (seg4<<1)|(bit?1:0)) : (seg1 = (seg1<<1)|(bit?1:0)); }
      //else if (x == 29 || x == 39 || x == 40) { if (bit) b_button = true; }
      else if (x == 29) { if (bit) b_button = true; }
      else if (x == 36) b_filter1    = bit;
      else if (x == 37) b_filter2    = bit;
      else if (x == 39) b_heater     = bit;
      else if (x == 40) b_set_heat   = bit;
      else if (x == 42) b_blower     = bit;
      else if (x == 43) b_set_filter = bit;
      else if (x == 44) b_filtration = bit;
      else if (x == 47) b_light      = bit;
      else if (x == 48) b_pump1      = bit;
      else if (x == 49) b_pump2      = bit;
      else if (x == 50) b_stop       = bit;
      else if (x == 55) b_time       = bit;
      else if (x == 56) b_set_time   = bit;
      else if (x == 58) b_start      = bit;
      else if (x == 59) b_std        = bit;
      else if (x == 60) b_eco        = bit;
      else if (x == 61) b_pm         = bit;
      else if (x == 63) b_am         = bit;
    }
    // ── Log des bits non mappés — affiche uniquement si changement ────────────
    // Bits connus : 1-28 (LCD), 29,39,40 (button), 36,37,42,43,44,47,48,49,
    //              50,55,56,57,58,59,60,61,63
    // Bits à surveiller : tous les autres entre 29 et 71
    {
      static uint8_t prev_raw[BALBOA_BUFFER_SIZE] = {};
      static bool first_run = true;

      const int known[] = {
        29,36,37,39,40,42,43,44,47,48,49,50,55,56,57,58,59,60,61,63
      };
      auto is_known = [&](int x) {
        if (x >= 1 && x <= 28) return true;  // LCD segments
        for (int k : known) if (k == x) return true;
        return false;
      };

      for (int x = 29; x <= (int)BALBOA_DISPLAY_BITS; x++) {
        if (is_known(x)) continue;
        uint8_t cur = buf[x];
        if (first_run || cur != prev_raw[x]) {
          ESP_LOGI(TAG, "bit[%02d] = %d", x, cur);
          prev_raw[x] = cur;
        }
      }
      // Log aussi les bits connus qui changent (pour débogage complet)
      for (int k : known) {
        uint8_t cur = buf[k];
        if (first_run || cur != prev_raw[k]) {
          const char* name = "";
          switch(k) {
            case 29: name="button(29)"; break;
            case 36: name="filter1";   break;
            case 37: name="filter2";   break;
            case 39: name="button(39)";break;
            case 40: name="button(40)";break;
            case 42: name="blower";    break;
            case 43: name="set_filter";break;
            case 44: name="filtration";break;
            case 47: name="light";     break;
            case 48: name="pump1";     break;
            case 49: name="pump2";     break;
            case 50: name="stop";      break;
            case 55: name="time";      break;
            case 56: name="set_time";  break;
            case 57: name="heater";    break;
            case 58: name="start";     break;
            case 59: name="std";       break;
            case 60: name="eco";       break;
            case 61: name="pm";        break;
            case 63: name="am";        break;
          }
          ESP_LOGD(TAG, "bit[%02d] %-12s = %d", k, name, cur);
          prev_raw[k] = cur;
        }
      }
      first_run = false;
    }
    last_seg1_ = seg1; last_seg2_ = seg2; last_seg3_ = seg3; last_seg4_ = seg4;

    // ── Throttling ────────────────────────────────────────────────────────────
    uint32_t now_ms = millis();
    bool force = (now_ms - last_publish_ms_) >= PUBLISH_INTERVAL_MS;
    if (force) last_publish_ms_ = now_ms;

    #define PUB_BIN(sensor, val, cache) \
      if (sensor && (val != cache)) { sensor->publish_state(val); cache = val; }

    PUB_BIN(heater_sensor_,     b_heater,     last_heater_)
    PUB_BIN(pump1_sensor_,      b_pump1,      last_pump1_)
    PUB_BIN(pump2_sensor_,      b_pump2,      last_pump2_)
    PUB_BIN(blower_sensor_,     b_blower,     last_blower_)
    PUB_BIN(light_sensor_,      b_light,      last_light_)
    PUB_BIN(filtration_sensor_, b_filtration, last_filtration_)
    PUB_BIN(filter1_sensor_,    b_filter1,    last_filter1_)
    PUB_BIN(filter2_sensor_,    b_filter2,    last_filter2_)
    PUB_BIN(am_sensor_,         b_am,         last_am_)
    PUB_BIN(pm_sensor_,         b_pm,         last_pm_)
    PUB_BIN(filter_start_,      b_start,      last_filter_start_)
    PUB_BIN(filter_stop_,       b_stop,       last_filter_stop_)
    PUB_BIN(set_time_sensor_,   b_set_time,   last_set_time_)
    PUB_BIN(set_filter_sensor_, b_set_filter, last_set_filter_)
    #undef PUB_BIN

    // ── Décodage LCD ─────────────────────────────────────────────────────────
    std::string d1 = lcd_char(seg1);
    std::string d2 = lcd_char(seg2);
    std::string d3 = lcd_char(seg3);
    std::string d4 = lcd_char(seg4);

    // ── b_time prioritaire (10h/11h/12h ont seg1 != 0) ───────────────────────
    if (b_time && !b_filter1 && !b_filter2 && !b_start && !b_stop) {
      std::string lcd_str = d1 + d2 + ":" + d3 + d4;
      if (lcd_sensor_ && (lcd_str != last_lcd_)) {
      lcd_sensor_->publish_state(lcd_str);
      last_lcd_ = lcd_str;
      //notify_climate_();
      }  
      bool hour_ok = (d2.size() == 1 && isdigit(d2[0])) &&
                     (d1 == " " || (d1.size() == 1 && isdigit(d1[0])));
      bool min_ok  = (d3.size() == 1 && isdigit(d3[0])) &&
                     (d4.size() == 1 && isdigit(d4[0]));
      if (hour_ok && min_ok) {
        int hour   = (d1 == " " ? 0 : (d1[0] - '0')) * 10 + (d2[0] - '0');
        int minute = (d3[0] - '0') * 10 + (d4[0] - '0');
        if (hour <= 23 && minute <= 59) {
          // Toujours mettre à jour spa_hour_/minute_ (utile pour TIME_SYNC_WAIT_READ)
          spa_hour_   = hour;
          spa_minute_ = minute;

          if(b_pm) spa_is_pm_ = true;
          else if(b_am) spa_is_pm_ = false;

          if (time_sensor_) {
            char tbuf[16];
            if (b_am)      snprintf(tbuf, sizeof(tbuf), "%02d:%02d AM", hour, minute);
            else if (b_pm) snprintf(tbuf, sizeof(tbuf), "%02d:%02d PM", hour, minute);
            else           snprintf(tbuf, sizeof(tbuf), "%02d:%02d", hour, minute);
            if(last_time_ != tbuf){
				time_sensor_->publish_state(tbuf);
				last_time_ = tbuf;
			}
          }
        }
      }

    } else if (b_filter1 && b_start) {
      std::string t = decode_filter_time_(d1, d2, d3, d4);
      if (lcd_sensor_ && (t != last_lcd_)) {
      lcd_sensor_->publish_state(t);
      last_lcd_ = t;
      }  
      if (!t.empty() && filter1_start_sensor_) {
        filter1_start_sensor_->publish_state(t);
        ESP_LOGI(TAG, "Filter 1 Start : %s", t.c_str());
      }
    } else if (b_filter1 && b_stop) {
      std::string t = decode_filter_time_(d1, d2, d3, d4);
      if (lcd_sensor_ && (t != last_lcd_)) {
      lcd_sensor_->publish_state(t);
      last_lcd_ = t;
      }  
      if (!t.empty() && filter1_stop_sensor_) {
        filter1_stop_sensor_->publish_state(t);
        ESP_LOGI(TAG, "Filter 1 Stop  : %s", t.c_str());
      }
    } else if (b_filter2 && b_start) {
      std::string t = decode_filter_time_(d1, d2, d3, d4);
      if (lcd_sensor_ && (t != last_lcd_)) {
      lcd_sensor_->publish_state(t);
      last_lcd_ = t;
      }  
      if (!t.empty() && filter2_start_sensor_) {
        filter2_start_sensor_->publish_state(t);
        ESP_LOGI(TAG, "Filter 2 Start : %s", t.c_str());
      }
    } else if (b_filter2 && b_stop) {
      std::string t = decode_filter_time_(d1, d2, d3, d4);
      if (lcd_sensor_ && (t != last_lcd_)) {
      lcd_sensor_->publish_state(t);
      last_lcd_ = t;
      }  
      if (!t.empty() && filter2_stop_sensor_) {
        filter2_stop_sensor_->publish_state(t);
        ESP_LOGI(TAG, "Filter 2 Stop  : %s", t.c_str());
      }

    } else if(b_set_time && (b_filter1 || b_filter2 || b_start || b_stop)) {
      std::string t = decode_filter_time_(d1, d2, d3, d4);
      if (lcd_sensor_ && (t != last_lcd_)) {
      lcd_sensor_->publish_state(t);
      last_lcd_ = t;
      }  
    } else if(b_set_time) {
      std::string t = decode_filter_time_(d1, d2, d3, d4);
      if (lcd_sensor_ && (t != last_lcd_)) {
      lcd_sensor_->publish_state(t);
      last_lcd_ = t;
      }  
    } else if (seg1 == 0) {
      std::string raw = d1 + d2 + d3 + d4;

      if      (raw == " 5LP")             display_mode_ = "Sleep";
      else if (raw == " Ecn" || b_eco)    display_mode_ = "Eco";
      else if (raw == " 5td" || b_std)    display_mode_ = "Standard";
      else if (raw == " 1CE")             display_mode_ = "Ice";
      else                                display_mode_ = raw;

      if (lcd_sensor_ && (raw != last_lcd_)) {
        lcd_sensor_->publish_state(raw);
        last_lcd_ = raw;
      }
      if (mode_sensor_ && (display_mode_ != last_mode_ )) {
        mode_sensor_->publish_state(display_mode_);
        last_mode_ = display_mode_;
        notify_climate_();
      }

    } else {
      bool digits_ok =
          (d1.size() == 1 && isdigit(d1[0])) &&
          (d2.size() == 1 && isdigit(d2[0])) &&
          (d3.size() == 1 && isdigit(d3[0])) &&
		  (d4 == "C");

      if (digits_ok) {
        float temp =
            (10.0f * (d1[0] - '0')) +
            (1.0f  * (d2[0] - '0')) +
            (0.1f  * (d3[0] - '0'));

        if (temp >= 0.0f && temp <= 45.0f) {
          std::string lcd_str = d1 + d2 + "." + d3 + d4;
          if (lcd_sensor_ && (lcd_str != last_lcd_)) {
            lcd_sensor_->publish_state(lcd_str);
            last_lcd_ = lcd_str;
            //notify_climate_();
          }

          if (b_set_heat) {
            set_temperature_ = temp;
            if (set_temp_sensor_ && (temp != last_set_temp_)) {
              set_temp_sensor_->publish_state(set_temperature_);
              last_set_temp_ = temp;
              notify_climate_();
            }
          } else {
            water_temperature_ = temp;
            if (!spa_ready_) {
              spa_ready_ = true;
              ESP_LOGI(TAG, "Spa prêt (temp eau=%.1f°C)", water_temperature_);
            }
            if (water_temp_sensor_ && (temp != last_water_temp_ )) {
              water_temp_sensor_->publish_state(water_temperature_);
              last_water_temp_ = temp;
              notify_climate_();
            }
          }
        }
      }
    }
  }

  void notify_climate_();  // défini après BalboaClimate

  std::string decode_filter_time_(const std::string &d1, const std::string &d2,
                                  const std::string &d3, const std::string &d4) {
    bool s1_ok = (d1 == " " || (d1.size() == 1 && isdigit(d1[0])));
    bool s234_ok = (d2.size() == 1 && isdigit(d2[0])) &&
                   (d3.size() == 1 && isdigit(d3[0])) &&
                   (d4.size() == 1 && isdigit(d4[0]));
    if (!s1_ok || !s234_ok) return "";
    int hour   = ((d1 == " " ? 0 : (d1[0] - '0')) * 10) + (d2[0] - '0');
    int minute = ((d3[0] - '0') * 10) + (d4[0] - '0');
    if (hour > 23 || minute > 59) return "";
    char buf[6];
    snprintf(buf, sizeof(buf), "%02d:%02d", hour, minute);
    return std::string(buf);
  }

  void apply_target_temperature_(float target) {
    float diff = target - set_temperature_;
    if (diff < 0)      update_temp_dir_ = 1;
    else if (diff > 0) update_temp_dir_ = 2;
    else               update_temp_dir_ = 0;
    update_temp_presses_ = 1 + (int)(fabsf(diff) * 2.0f);
    ESP_LOGI(TAG, "Consigne → %.1f°C (spa=%.1f°C, %d appuis)",
             target, set_temperature_, update_temp_presses_);
  }
};

}  // namespace balboa_spa
}  // namespace esphome

// ─── BalboaClimate ────────────────────────────────────────────────────────────
#include "esphome/components/climate/climate.h"

namespace esphome {
namespace balboa_spa {

class BalboaClimate : public climate::Climate, public Component {
 public:
  void set_balboa(BalboaSpaComponent *balboa) { balboa_ = balboa; }

  void setup() override {}
  float get_setup_priority() const override { return setup_priority::DATA; }

  climate::ClimateTraits traits() override {
    auto traits = climate::ClimateTraits();
    traits.add_feature_flags(climate::CLIMATE_SUPPORTS_CURRENT_TEMPERATURE);
    traits.set_visual_min_temperature(26.0f);
    traits.set_visual_max_temperature(40.0f);
    traits.set_visual_temperature_step(0.5f);
    traits.set_supported_modes({
      climate::CLIMATE_MODE_HEAT,
      climate::CLIMATE_MODE_OFF,
    });
    traits.set_supported_presets({
      climate::CLIMATE_PRESET_COMFORT,
      climate::CLIMATE_PRESET_ECO,
      climate::CLIMATE_PRESET_SLEEP,
    });
    return traits;
  }

  void control(const climate::ClimateCall &call) override {
    if (call.get_mode().has_value()) {
      this->mode = *call.get_mode();
      this->publish_state();
    }
    if (call.get_preset().has_value()) {
      auto preset = *call.get_preset();
      this->preset = preset;
      this->publish_state();
      if (balboa_) {
        balboa_->press_mode();
        ESP_LOGI("balboa_climate", "Preset demandé : %d → appui Mode", (int)preset);
      }
    }
    if (call.get_target_temperature().has_value()) {
      float target = *call.get_target_temperature();
      this->target_temperature = target;
      this->publish_state();
      if (balboa_ && this->mode != climate::CLIMATE_MODE_OFF) {
        balboa_->set_target_temperature(target);
      }
    }
  }

  void update_from_spa(float water_temp, float set_temp, const std::string &spa_mode) {
    this->current_temperature = water_temp;
    if (!balboa_ || balboa_->get_pending_presses() == 0) {
      this->target_temperature = set_temp;
    }
    this->mode = climate::CLIMATE_MODE_HEAT;
    if      (spa_mode == "Standard")                   this->preset = climate::CLIMATE_PRESET_COMFORT;
    else if (spa_mode == "Eco")                        this->preset = climate::CLIMATE_PRESET_ECO;
    else if (spa_mode == "Sleep" || spa_mode == "Ice") this->preset = climate::CLIMATE_PRESET_SLEEP;
    this->publish_state();
  }

 protected:
  BalboaSpaComponent *balboa_ = nullptr;
};

inline void BalboaSpaComponent::notify_climate_() {
  if (climate_) {
    climate_->update_from_spa(water_temperature_, set_temperature_, display_mode_);
  }
}

}  // namespace balboa_spa
}  // namespace esphome
