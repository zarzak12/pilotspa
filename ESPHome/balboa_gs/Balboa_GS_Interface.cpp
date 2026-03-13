
#include "Balboa_GS_Interface.h"


byte BalboaInterface::clockPin;
byte BalboaInterface::displayPin;
byte BalboaInterface::buttonPin;
bool BalboaInterface::displayDataBufferOverflow;
bool BalboaInterface::writeDisplayData = false;
bool BalboaInterface::writeMode = false;
bool BalboaInterface::writeTempUp = false;
bool BalboaInterface::writeTempDown = false;
bool BalboaInterface::writeLight = false;
bool BalboaInterface::writePump1 = false;
bool BalboaInterface::writePump2 = false;
bool BalboaInterface::writeTime = false;
bool BalboaInterface::writeBlower = false;
unsigned long BalboaInterface::clockInterruptTime;
int BalboaInterface::clockBitCounter;
volatile byte BalboaInterface::displayDataBuffer[displayDataBufferSize];
byte BalboaInterface::dataIndex;
volatile bool BalboaInterface::displayDataBufferReady;


BalboaInterface::BalboaInterface(byte setClockPin, byte setReadPin, byte setWritePin) {
  clockPin = setClockPin;
  displayPin = setReadPin;
  buttonPin = setWritePin;
}

void BalboaInterface::begin() {
  pinMode(clockPin, INPUT);
  pinMode(displayPin, INPUT);
  pinMode(buttonPin, OUTPUT);
  digitalWrite(buttonPin, LOW);

  attachInterrupt(clockPin, clockPinInterrupt, CHANGE);
}

void BalboaInterface::stop() {

  detachInterrupt(digitalPinToInterrupt(clockPin));
}

bool BalboaInterface::loop() {

  if (displayDataBufferReady) {

    noInterrupts();              // <<< protection buffer

    // Decode data once available
    decodeDisplayData();

    displayDataBufferReady = false;

    interrupts();                // <<< réactive les interruptions

    // Get setTemperature if not known
    if (setTemperature == 0) {
      writeDisplayData = true;
      writeTempUp = true;
    }

    // Get setTime if not known
    if (timeDifference && !displaySetTime && !displaySetFilter && !displayStart && !displayStop && !displayTime && !traitementEnCours) {
      writeDisplayData = true;
      writeTime = true;
      timeDifference = false;
    }
    
    // Update temperature
    if (updateTempButtonPresses > 0) {
      // Check if enough time has passed since the last button press
      if (millis() - buttonPressTimerPrevMillis > buttonPressTimerMillis) {
        // Update the previous button press timestamp
        buttonPressTimerPrevMillis = millis();
    
        // Check the direction of temperature update
        if (updateTempDirection == 1) {
          // If the direction is down, set flags for updating display and decreasing temperature
          writeDisplayData = true;
          writeTempDown = true;
        } else if (updateTempDirection == 2) {
          // If the direction is up, set flags for updating display and increasing temperature
          writeDisplayData = true;
          writeTempUp = true;
        }
    
        // Decrease the remaining button presses count
        updateTempButtonPresses--;
      }
    }

    // Update mode
    if (updateModeButtonPresses > 0) {
    
      if (millis() - buttonPressTimerPrevMillis > buttonPressTimerMillis) {
    
        buttonPressTimerPrevMillis = millis();
    
        writeDisplayData = true;
        writeMode = true;
    
        updateModeButtonPresses--;
      }
    }

    // Get Time
    if (callTime) {
      // Check if enough time has passed since the last button press
      if (millis() - buttonPressTimerPrevMillis > buttonPressTimerMillis) {
        // Update the previous button press timestamp
        buttonPressTimerPrevMillis = millis();
    
        // Check if there are remaining button presses to get the time
        if (callTimeButtonPresses > 0) {
          // Set flags to indicate that display data and time should be updated
          writeDisplayData = true;
          writeTime = true;
    
          // Decrease the count of remaining button presses to get the time
          callTimeButtonPresses--;
        } else {
          // If no more button presses are required, reset flags and exit time retrieval mode
          callTime = false;
          traitementEnCours = false;
        }        
      }
    }


    // Obtention du Filtre de l'Heure
    if (callTimeFilter) {
      // Vérifie si suffisamment de temps s'est écoulé depuis la dernière pression de bouton
      if (millis() - buttonPressTimerPrevMillis > buttonPressTimerMillis) {
        // Met à jour le chronomètre de la dernière pression de bouton
        buttonPressTimerPrevMillis = millis();
    
        // Vérifie s'il reste plusieurs pressions de bouton pour mettre à jour le l'heure du filtre
        if (updateTimeFTButtonPresses > 1) {
          // Active les indicateurs pour indiquer que les données d'affichage et l'heure doivent être mises à jour
          writeDisplayData = true;
          writeTime = true;
    
          // Diminue le nombre de pressions de bouton restantes pour le filtre de l'heure
          updateTimeFTButtonPresses--;
        } else if (updateTimeFTButtonPresses == 1) {
          // Vérifie s'il ne reste qu'une pression de bouton pour le filtre de l'heure
          if (updateModeFTButtonPresses > 0) {
            // Active les indicateurs pour indiquer que les données d'affichage et le mode doivent être mis à jour
            writeDisplayData = true;
            writeMode = true;
    
            // Diminue le nombre de pressions de bouton restantes pour le mode
            updateModeFTButtonPresses--;
          } else if (updateModeFTButtonPresses == 0) {
            // Si aucune autre pression de bouton pour le mode n'est requise, met à jour l'heure et quitte le mode de filtre de l'heure
            writeDisplayData = true;
            writeTime = true;
    
            // Diminue le nombre de pressions de bouton restantes pour le filtre de l'heure
            updateTimeFTButtonPresses--;
    
            // Réinitialise les indicateurs et quitte le mode de filtre de l'heure
            callTimeFilter = false;
            traitementEnCours = false;
          }
        }
      }
    }


    // Set 5h /6h AM filter 1
    if (filter1) {

      if (millis() - buttonPressTimerPrevMillis > buttonPressTimerMillis) {

        buttonPressTimerPrevMillis = millis();

        if (updateTimeFT1ButtonPresses > 1) {
          writeDisplayData = true;
          writeTime = true;
          updateTimeFT1ButtonPresses--;
        } else if (updateTimeFT1ButtonPresses == 1){
          if (updateModeFT1ButtonPresses > 2) {
            writeDisplayData = true;
            writeMode = true;
            updateModeFT1ButtonPresses--;
          } else if (updateModeFT1ButtonPresses == 2) {
            if(impulsionsFilter1_1 > 0){
              writeDisplayData = true;
              writeTempDown = true;
              impulsionsFilter1_1--;
            }else{
              writeDisplayData = true;
              writeMode = true;
              updateModeFT1ButtonPresses--;
            }
          }
          else if (updateModeFT2ButtonPresses == 1) {
            if(impulsionsFilter1_2 > 0){
              writeDisplayData = true;
              writeTempDown = true;
              impulsionsFilter1_2--;
            }else{
              writeDisplayData = true;
              writeTime = true;
              updateModeFT1ButtonPresses--;
              updateTimeFT1ButtonPresses--;
              filter1=false;
              traitementEnCours = false;
            }
          }
        }

      }
    }

    // Set 10h /11h30 PM filter 2
    if (filter2) {

      if (millis() - buttonPressTimerPrevMillis > buttonPressTimerMillis) {

        buttonPressTimerPrevMillis = millis();

        if (updateTimeFT2ButtonPresses > 1) {
          writeDisplayData = true;
          writeTime = true;
          updateTimeFT2ButtonPresses--;
        } else if (updateTimeFT2ButtonPresses == 1){
          if (updateModeFT2ButtonPresses > 2) {
            writeDisplayData = true;
            writeMode = true;
            updateModeFT2ButtonPresses--;
          } else if (updateModeFT2ButtonPresses == 2) {
            if(impulsionsFilter2_1 > 0){
              writeDisplayData = true;
              writeTempUp = true;
              impulsionsFilter2_1--;
            }else{
              writeDisplayData = true;
              writeMode = true;
              updateModeFT2ButtonPresses--;
            }
          }
          else if (updateModeFT2ButtonPresses == 1) {
            if(impulsionsFilter2_2 > 0){
              writeDisplayData = true;
              writeTempUp = true;
              impulsionsFilter2_2--;
            }else{
              writeDisplayData = true;
              writeTime = true;
              updateModeFT2ButtonPresses--;
              updateTimeFT2ButtonPresses--;
              filter2=false;
              traitementEnCours = false;
            }
          }
        }

      }
    }

     // Update time
    if (!heureOK) {

      if (millis() - buttonPressTimerPrevMillis > buttonPressTimerMillis) {

        buttonPressTimerPrevMillis = millis();
        if(differenceMinutes > 0) {
          if(!timePush){
            writeDisplayData = true;
            writeTime = true;
            timePush=true;
          } else {
            if(!modePush){
              writeDisplayData = true;
              writeMode = true;
              modePush=true;
            } else {
              if (updateHeureDirection == 1){
                writeDisplayData = true;
                writeTempDown = true;
              } else if (updateHeureDirection == 2) {
                writeDisplayData = true;
                writeTempUp = true;
              }
      
              differenceMinutes--;             
            }
          }
        } else {
          //si le traitement est terminé on sort de l'ecran de paramétrage de l'heure
          writeDisplayData = true;
          writeTime = true;
          modePush=false;
          timePush=false;
          heureOK=true;
          traitementEnCours = false;       
        }
      }
    }
    
    return true;
    
  }

  return false;
}


void BalboaInterface::updateTemperature(float Temperature) {

  float updateTempDifference = Temperature - setTemperature;

  if (updateTempDifference < 0) { updateTempDirection = 1; }  // Temp down
  else if (updateTempDifference > 0) {
    updateTempDirection = 2;
  }                                                                 // Temp up
  else if (updateTempDifference == 0) { updateTempDirection = 0; }  // no change

  updateTempButtonPresses = 1 + (abs(updateTempDifference) * 2);  // calculate how many times the "button" should be pressed
                                                                  // every button press = 0.5 and the first is to enter the menu
}

void BalboaInterface::getTime(){
  if(!traitementEnCours){
    callTime = true;
    callTimeButtonPresses = 2;
  }
}

float convertirEnHeuresDecimales(int heures, int minutes) {
    return heures + (minutes / 60.0);
}

void BalboaInterface::calculImpultionfilter1_1(){
    int diffDeb = 0;
  //8h -> 5h = 60 * 3 * 2 = 360
  //impulsionsFilter1_1=360;
  if(heureFilter1Deb > 8){
    diffDeb = heureFilter1Deb - 8;
  } else {
    diffDeb = 8 - heureFilter1Deb;
  }
  float heure = 0;
  heure = convertirEnHeuresDecimales(diffDeb, minuteFilter1Deb);
  impulsionsFilter1_1=heure * 60 * 2;
}

void BalboaInterface::calculImpultionfilter1_2(){
    int diffDeb = 0;
  //10h -> 6h = 60 * 4 * 2 = 480
  //impulsionsFilter1_2=480;
  if(heureFilter1Fin > 10){
    diffDeb = heureFilter1Fin - 10;
  } else {
    diffDeb = 10 - heureFilter1Fin;
  }
  float heure = 0;
  heure = convertirEnHeuresDecimales(diffDeb, minuteFilter1Fin);
  impulsionsFilter1_2=heure * 60 * 2;
}

void BalboaInterface::setFilter1(){
  if(heureFilter1Fin != 9 && !traitementEnCours){
    filter1 = true;
    updateTimeFT1ButtonPresses = 2;
    updateModeFT1ButtonPresses = 3;
    //8h -> 5h = 60 * 3 * 2 = 360
    impulsionsFilter1_1=360;
    //10h -> 6h = 60 * 4 * 2 = 480
    impulsionsFilter1_2=480;
    traitementEnCours = true;
  }  
}

void BalboaInterface::setFilter2(){
  if(heureFilter2Deb != 10 && heureFilter2Fin != 11 && !traitementEnCours){
    filter2 = true;
    updateTimeFT2ButtonPresses = 2;
    updateModeFT2ButtonPresses = 6;
    //8h -> 10h : 60 * 2 * 2 = 240
    float heureDecim = 0;
    heureDecim = convertirEnHeuresDecimales(2,0);
    impulsionsFilter2_1=heureDecim * 60 * 2;
    //10h -> 11h30 : 60 * 1,5 * 2 = 180
    heureDecim = convertirEnHeuresDecimales(1,45);
    impulsionsFilter2_2=heureDecim * 60 * 2;
    traitementEnCours = true;
  }
}

void BalboaInterface::getFilterTime(){
  if(!traitementEnCours){
    callTimeFilter = true;
    updateTimeFTButtonPresses = 2;
    updateModeFTButtonPresses = 5;
    traitementEnCours = true;
  }  
}

void BalboaInterface::updateTimeFilter(int heureSet, int minuteSet, int mode) {  

  if(!traitementEnCours){
    updateTimeFilterBool=true;
    modePush=false;
    timePush=false;
    differenceMinutes=0;
  
    traitementEnCours = true;
  
    int heureSave;
    int minuteSave;
  
    updateTimeUFTButtonPresses = 2;
    
    if (mode == 1){
      updateModeUFTButtonPresses = 2;
      heureSave=heureFilter1Deb;
      minuteSave=minuteFilter1Deb;
    } else if (mode == 2){
      updateModeUFTButtonPresses = 3;
      heureSave=heureFilter1Fin;
      minuteSave=minuteFilter1Fin;
    } else if (mode == 3){
      updateModeUFTButtonPresses = 4;
      heureSave=heureFilter2Deb;
      minuteSave=minuteFilter2Deb;
    } else if (mode == 4){
      updateModeUFTButtonPresses = 5;
      heureSave=heureFilter2Fin;
      minuteSave=minuteFilter2Fin;
    }
  
    int heurePeriph = convertirHeureEnMinutes(heureSave, minuteSave);
  
    if (heurePeriph != heureSet * 60 + minuteSet) {
      differenceMinutesUFT = ((heureSet * 60 + minuteSet) - heurePeriph) * 2; //nombre de minutes à modifier fois 2 car pour 1 minute il y a un appuis pour modifier et 1 pour stopper.
  
      if (differenceMinutesUFT > 0) {
        updateHeureDirectionUFT = 2;      
      } else if (differenceMinutesUFT < 0) {
        updateHeureDirectionUFT = 1;
        differenceMinutesUFT=-differenceMinutesUFT;
      } else {
        updateHeureDirectionUFT = 0; 
        updateTimeFilterBool=false;
        updateTimeUFTButtonPresses=0;
        updateModeUFTButtonPresses=0;
      }
    }
  }
}

int BalboaInterface::convertirHeureEnMinutes(int heureSpa, int minuteSpa) {

  // Convertir l'heure au format 12h en 24h si nécessaire
  if (displayPM && heureSpa < 12) {
    heureSpa = heureSpa;
  } else if (displayAM && heureSpa == 12) {
    heureSpa = 0;
  }

  return heureSpa * 60 + minuteSpa;
}

void BalboaInterface::updateTime(int heureSet, int minuteSet) {  

  //getTime(); 
  heureOK=false;
  modePush=false;
  timePush=false;
  differenceMinutes=0;

  int heurePeriph = convertirHeureEnMinutes(heureLue, minuteLue);

  // Convertir l'heure au format 24h en 12h si nécessaire
  if (heureSet > 12) {
    heureSet -= 12;
  } 

  if (heurePeriph != heureSet * 60 + minuteSet) {
    differenceMinutes = ((heureSet * 60 + minuteSet) - heurePeriph) * 2; //nombre de minutes à modifier fois 2 car pour 1 minute il y a un appuis pour modifier et 1 pour stopper.

    if (differenceMinutes > 0) {
      updateHeureDirection = 2;
      traitementEnCours = true;
    } else if (differenceMinutes < 0) {
      updateHeureDirection = 1;
      differenceMinutes=-differenceMinutes;
      traitementEnCours = true;
    } else {
      updateHeureDirection = 0; 
      heureOK=true;
    }
  }
}

void BalboaInterface::updateMode(String Mode) {

  updateModeButtonPresses = 0;

  if(displayStandardMode && Mode != stdModeProg)
  {
    if(Mode == ecoModeProg) {
      updateModeButtonPresses = 1;
    }else if (Mode == slpModeProg){
      updateModeButtonPresses = 2;
    }
  }else if(displayEcoMode && Mode != ecoModeProg)
  {
    if(Mode == slpModeProg) {
      updateModeButtonPresses = 1;
    }else if (Mode == stdModeProg){
      updateModeButtonPresses = 2;
    }
  }else if(displayMode == " SLP" && Mode != slpModeProg)
  {
    if(Mode == stdModeProg) {
      updateModeButtonPresses = 1;
    }else if (Mode == ecoModeProg){
      updateModeButtonPresses = 2;
    }
  }else{
    updateModeButtonPresses = 0;
  }
}


void BalboaInterface::decodeDisplayData() {

  LCD_segment_1 = 0;
  LCD_segment_2 = 0;
  LCD_segment_3 = 0;
  LCD_segment_4 = 0;

  LCD_display = "";

  for (int x = 0; x <= displayDataBits; x++) {

    if (x == 0) {
      if (displayDataBuffer[x] == 1) {
        displayBit0 = true;
      } else displayBit0 = false;
    } else if (x > 0 && x <= 7) {
      if (displayBit0) {
        LCD_segment_1 <<= 1;
        if (displayDataBuffer[x] == 1) {
          LCD_segment_1 |= 1;
        } else LCD_segment_1 |= 0;
      } else {
        LCD_segment_4 <<= 1;
        if (displayDataBuffer[x] == 1) {
          LCD_segment_4 |= 1;
        } else LCD_segment_4 |= 0;
      }
    } else if (x > 7 && x <= 14) {
      if (displayBit0) {
        LCD_segment_2 <<= 1;
        if (displayDataBuffer[x] == 1) {
          LCD_segment_2 |= 1;
        } else LCD_segment_2 |= 0;
      } else {
        LCD_segment_3 <<= 1;
        if (displayDataBuffer[x] == 1) {
          LCD_segment_3 |= 1;
        } else LCD_segment_3 |= 0;
      }
    } else if (x > 14 && x <= 21) {
      if (displayBit0) {
        LCD_segment_3 <<= 1;
        if (displayDataBuffer[x] == 1) {
          LCD_segment_3 |= 1;
        } else LCD_segment_3 |= 0;
      } else {
        LCD_segment_2 <<= 1;
        if (displayDataBuffer[x] == 1) {
          LCD_segment_2 |= 1;
        } else LCD_segment_2 |= 0;
      }
    } else if (x > 21 && x <= 28) {
      if (displayBit0) {
        LCD_segment_4 <<= 1;
        if (displayDataBuffer[x] == 1) {
          LCD_segment_4 |= 1;
        } else LCD_segment_4 |= 0;
      } else {
        LCD_segment_1 <<= 1;
        if (displayDataBuffer[x] == 1) {
          LCD_segment_1 |= 1;
        } else LCD_segment_1 |= 0;
      }
    } else if (x == 29) {
      if (displayDataBuffer[x] == 1) {
        displayButton = true;
      } else displayButton = false;
    } else if (x == 30) {
      if (displayDataBuffer[x] == 1) {
        displayBit30 = true;
      } else displayBit30 = false;
    } else if (x == 31) {
      if (displayDataBuffer[x] == 1) {
        displayBit31 = true;
      } else displayBit31 = false;
    } else if (x == 32) {
      if (displayDataBuffer[x] == 1) {
        displayBit32 = true;
      } else displayBit32 = false;
    } else if (x == 33) {
      if (displayDataBuffer[x] == 1) {
        displayBit33 = true;
      } else displayBit33 = false;
    } else if (x == 34) {
      if (displayDataBuffer[x] == 1) {
        displayBit34 = true;
      } else displayBit34 = false;
    } else if (x == 35) {
      if (displayDataBuffer[x] == 1) {
        displayBit35 = true;
      } else displayBit35 = false;
    } else if (x == 36) {
      if (displayDataBuffer[x] == 1) {
        displayFilter1 = true;
      } else displayFilter1 = false;
    } else if (x == 37) {
      if (displayDataBuffer[x] == 1) {
        displayFilter2 = true;
      } else displayFilter2 = false;
    } else if (x == 38) {
      if (displayDataBuffer[x] == 1) {
        displayBit38 = true;
      } else displayBit38 = false;
    } else if (x == 39) {
      if (displayDataBuffer[x] == 1) {
        displayButton = true;
      } else displayButton = false;
    } else if (x == 40) {
      if (displayDataBuffer[x] == 1) {
        displayButton = true;
      } else displayButton = false;
    } else if (x == 41) {
      if (displayDataBuffer[x] == 1) {
        displayBit41 = true;
      } else displayBit41 = false;
    } else if (x == 42) {
      if (displayDataBuffer[x] == 1) {
        displayBlower = true;
      } else displayBlower = false;
    } else if (x == 43) {
      if (displayDataBuffer[x] == 1) {
        displaySetFilter = true;
      } else displaySetFilter = false;
    } else if (x == 44) {
      if (displayDataBuffer[x] == 1) {
        displayFiltration = true;
      } else displayFiltration = false;
    } else if (x == 45) {
      if (displayDataBuffer[x] == 1) {
        displayBit45 = true;
      } else displayBit45 = false;
    } else if (x == 46) {
      if (displayDataBuffer[x] == 1) {
        displayBit46 = true;
      } else displayBit46 = false;
    } else if (x == 47) {
      if (displayDataBuffer[x] == 1) {
        displayLight = true;
      } else displayLight = false;
    } else if (x == 48) {
      if (displayDataBuffer[x] == 1) {
        displayPump1 = true;
      } else displayPump1 = false;
    } else if (x == 49) {
      if (displayDataBuffer[x] == 1) {
        displayPump2 = true;
      } else displayPump2 = false;
    } else if (x == 50) {
      if (displayDataBuffer[x] == 1) {
        displayStop = true;
      } else displayStop = false;
    } else if (x == 51) {
      if (displayDataBuffer[x] == 1) {
        displayBit51 = true;
      } else displayBit51 = false;
    } else if (x == 52) {
      if (displayDataBuffer[x] == 1) {
        displayBit52 = true;
      } else displayBit52 = false;
    } else if (x == 53) {
      if (displayDataBuffer[x] == 1) {
        displayBit53 = true;
      } else displayBit53 = false;
    } else if (x == 54) {
      if (displayDataBuffer[x] == 1) {
        displayBit54 = true;
      } else displayBit54 = false;
    } else if (x == 55) {
      if (displayDataBuffer[x] == 1) {
        displayTime = true;
      } else displayTime = false;
    } else if (x == 56) {
      if (displayDataBuffer[x] == 1) {
        displaySetTime = true;
      } else displaySetTime = false;
    } else if (x == 57) {
      if (displayDataBuffer[x] == 1) {
        displayBit57 = true;
      } else displayBit57 = false;
    } else if (x == 58) {
      if (displayDataBuffer[x] == 1) {
        displayStart = true;
      } else displayStart = false;
    } else if (x == 59) {
      if (displayDataBuffer[x] == 1) {
        displayStandardMode = true;
      } else displayStandardMode = false;
    } else if (x == 60) {
      if (displayDataBuffer[x] == 1) {
        displayEcoMode = true;
      } else displayEcoMode = false;
    } else if (x == 61) {
      if (displayDataBuffer[x] == 1) {
        displayPM = true;
      } else displayPM = false;
    } else if (x == 62) {
      if (displayDataBuffer[x] == 1) {
        displayBit62 = true;
      } else displayBit62 = false;
    } else if (x == 63) {
      if (displayDataBuffer[x] == 1) {
        displayAM = true;
      } else displayAM = false;
    } else if (x == 64) {
      if (displayDataBuffer[x] == 1) {
        displayBit64 = true;
      } else displayBit64 = false;
    } else if (x == 65) {
      if (displayDataBuffer[x] == 1) {
        displayBit65 = true;
      } else displayBit65 = false;
    } else if (x == 66) {
      if (displayDataBuffer[x] == 1) {
        displayBit66 = true;
      } else displayBit66 = false;
    } else if (x == 67) {
      if (displayDataBuffer[x] == 1) {
        displayBit67 = true;
      } else displayBit67 = false;
    } else if (x == 68) {
      if (displayDataBuffer[x] == 1) {
        displayBit68 = true;
      } else displayBit68 = false;
    } else if (x == 69) {
      if (displayDataBuffer[x] == 1) {
        displayBit69 = true;
      } else displayBit69 = false;
    } else if (x == 70) {
      if (displayDataBuffer[x] == 1) {
        displayBit70 = true;
      } else displayBit70 = false;
    } else if (x == 71) {
      if (displayDataBuffer[x] == 1) {
        displayBit71 = true;
      } else displayBit71 = false;
    }
  }

  LCD_display_1 = lockup_LCD_character(LCD_segment_1);
  LCD_display_2 = lockup_LCD_character(LCD_segment_2);
  LCD_display_3 = lockup_LCD_character(LCD_segment_3);
  LCD_display_4 = lockup_LCD_character(LCD_segment_4);


  // check if temperature or something else is shown on LCD display

  // No temperature is shown
  if (LCD_segment_1 == 0) {
    LCD_display = LCD_display_1 + LCD_display_2 + LCD_display_3 + LCD_display_4;
    if (LCD_display == " 5LP") {
      displayMode = " SLP";
    } else if (LCD_display == " Ecn" || displayEcoMode) {
      displayMode = " Ecn";
    } else if (LCD_display == " 5td" || displayStandardMode) {
      displayMode = " Std";
    }else {
      int number = LCD_display.toInt();
      // Vérifiez si la conversion a réussi
      if (LCD_display != "" && !LCD_display.equals("0") && number == 0) {
          // La conversion a échoué, car la chaîne n'était pas un nombre entier valide
      } else {
          // La conversion a réussi, et 'number' contient la valeur entière
          if(number >= 0 && number <= 1259)
          {
            if(displaySetFilter && displayFilter1 && displayStart) {
              heureFilter1Deb = number / 100;
              minuteFilter1Deb = number % 100;
            } else if(displaySetFilter && displayFilter1 && displayStop) {
              heureFilter1Fin = number / 100;
              minuteFilter1Fin = number % 100;
            } else if(displaySetFilter && displayFilter2 && displayStart) {
              heureFilter2Deb = number / 100;
              minuteFilter2Deb = number % 100;
            } else if(displaySetFilter && displayFilter2 && displayStop) {
              heureFilter2Fin = number / 100;
              minuteFilter2Fin = number % 100;
            } else {
              heureLue = number / 100;      // Obtenez les heures (801 / 100 = 8)
              minuteLue = number % 100;    // Obtenez les minutes (801 % 100 = 1)
            }
          }
      }
    }
  }

  // Temperature is shown
  else {
    LCD_display = LCD_display_1 + LCD_display_2 + LCD_display_3 + LCD_display_4;
    int number = LCD_display.toInt();
    if (displayTime || displaySetTime || displayAM || displayPM || LCD_display.indexOf('C') == -1) {      
      // Vérifiez si la conversion a réussi
      if (LCD_display != "" && !LCD_display.equals("0") && number == 0) {
          // La conversion a échoué, car la chaîne n'était pas un nombre entier valide
      } else {
          // La conversion a réussi, et 'number' contient la valeur entière
          if(number >= 0 && number <= 1259 )
          {
            if(displaySetFilter && displayFilter1 && displayStart) {
              heureFilter1Deb = number / 100;
              minuteFilter1Deb = number % 100;
            } else if(displaySetFilter && displayFilter1 && displayStop) {
              heureFilter1Fin = number / 100;
              minuteFilter1Fin = number % 100;
            } else if(displaySetFilter && displayFilter2 && displayStart) {
              heureFilter2Deb = number / 100;
              minuteFilter2Deb = number % 100;
            } else if(displaySetFilter && displayFilter2 && displayStop) {
              heureFilter2Fin = number / 100;
              minuteFilter2Fin = number % 100;
            } else {
              heureLue = number / 100;      // Obtenez les heures (801 / 100 = 8)
              minuteLue = number % 100;    // Obtenez les minutes (801 % 100 = 1)
            }
          }
      }
    }
    else {
      float Temperature = (10 * LCD_display_1.toInt() + LCD_display_2.toInt() + 0.1 * LCD_display_3.toInt());

      // Vérifie si le bouton d'affichage est activé
      if (displayButton) {
          // Si le bouton est activé, enregistre la température
          setTemperature = Temperature;
      } else {
          waterTemperature = Temperature;
      }

      LCD_display = LCD_display_1 + LCD_display_2 + "." + LCD_display_3 + LCD_display_4;
    }
  }


  displayDataBufferReady = false;
  attachInterrupt(clockPin, clockPinInterrupt, CHANGE);
}

ICACHE_RAM_ATTR void BalboaInterface::clockPinInterrupt() {


  if (!displayDataBufferReady) {

    if ((micros() - clockInterruptTime) >= durationNewCycle) {  // New cycle detected
      dataIndex = 0;
      clockBitCounter = 0;
      displayDataBufferReady = false;
    }

    clockInterruptTime = micros();

    if (digitalRead(clockPin) == LOW) { digitalWrite(buttonPin, LOW); }

    if (digitalRead(clockPin) == HIGH) {


      // Write button data if requested

      /*#### Button data 

      |  Button   |  Decoding bit(0-3) | 
      | --------- | ------------------ |
      | Mode      |   1 0 0 0            |
      | Temp up   |   1 1 1 0            |
      | Temp down |   1 1 1 1            |  
      | Light     |   1 0 1 1            |
      | Pump 1    |   1 0 0 0            | 
      | Pump 2    |   1 0 1 0            |
      | Time      |   1 1 0 0            |
      | Blower    |   1 1 0 1            |*/

      if (writeDisplayData == true && clockBitCounter >= 72 && clockBitCounter <= 75) {

        if (clockBitCounter == 72) {

          if (writeMode) {
            digitalWrite(buttonPin, HIGH);
          } else if (writeTempUp) {
            digitalWrite(buttonPin, HIGH);
          } else if (writeTempDown) {
            digitalWrite(buttonPin, HIGH);
          } else if (writeLight) {
            digitalWrite(buttonPin, HIGH);
          } else if (writePump1) {
            digitalWrite(buttonPin, HIGH);
          } else if (writePump2) {
            digitalWrite(buttonPin, HIGH);
          } else if (writeTime) {
            digitalWrite(buttonPin, HIGH);
          }else if (writeBlower) {
            digitalWrite(buttonPin, HIGH);
          }
        }

        else if (clockBitCounter == 73) {

          if (writeMode) {
            digitalWrite(buttonPin, LOW);
          } else if (writeTempUp) {
            digitalWrite(buttonPin, HIGH);
          } else if (writeTempDown) {
            digitalWrite(buttonPin, HIGH);
          } else if (writeLight) {
            digitalWrite(buttonPin, LOW);
          } else if (writePump1) {
            digitalWrite(buttonPin, LOW);
          } else if (writePump2) {
            digitalWrite(buttonPin, LOW);
          } else if (writeTime) {
            digitalWrite(buttonPin, HIGH);
          } else if (writeBlower) {
            digitalWrite(buttonPin, HIGH);
          }
        }


        else if (clockBitCounter == 74) {

          if (writeMode) {
            digitalWrite(buttonPin, LOW);
          } else if (writeTempUp) {
            digitalWrite(buttonPin, HIGH);
          } else if (writeTempDown) {
            digitalWrite(buttonPin, HIGH);
          } else if (writeLight) {
            digitalWrite(buttonPin, HIGH);
          } else if (writePump1) {
            digitalWrite(buttonPin, LOW);
          } else if (writePump2) {
            digitalWrite(buttonPin, HIGH);
          } else if (writeTime) {
            digitalWrite(buttonPin, LOW);
          } else if (writeBlower) {
            digitalWrite(buttonPin, LOW);
          }
        }

        else if (clockBitCounter == 75) {

          if (writeMode) {
            digitalWrite(buttonPin, LOW);
          } else if (writeTempUp) {
            digitalWrite(buttonPin, LOW);
          } else if (writeTempDown) {
            digitalWrite(buttonPin, HIGH);
          } else if (writeLight) {
            digitalWrite(buttonPin, HIGH);
          } else if (writePump1) {
            digitalWrite(buttonPin, HIGH);
          } else if (writePump2) {
            digitalWrite(buttonPin, LOW);
          } else if (writeTime) {
            digitalWrite(buttonPin, LOW);
          } else if (writeBlower) {
            digitalWrite(buttonPin, HIGH);
          }

          writeMode = false;
          writeTempUp = false;
          writeTempDown = false;
          writeLight = false;
          writePump1 = false;
          writePump2 = false;
          writeBlower = false;
          writeTime = false;
        }
      }

      // Read display data

      if (clockBitCounter <= displayDataBits) {
        displayDataBuffer[dataIndex] = digitalRead(displayPin);
        dataIndex++;
      } else if (clockBitCounter == totalDataBits) {  // Total cycle has passed
        displayDataBufferReady = true;
        detachInterrupt(digitalPinToInterrupt(clockPin));
      } else if (clockBitCounter > totalDataBits) {
        displayDataBufferOverflow = true;
      }

      clockBitCounter++;
    }
  }
}

String BalboaInterface::lockup_LCD_character(int LCD_character) {


  switch (LCD_character) {
    case 0b0000000: return " "; break;
    case 0b1111110: return "0"; break;
    case 0b0110000: return "1"; break;
    case 0b1101101: return "2"; break;
    case 0b1111001: return "3"; break;
    case 0b0110011: return "4"; break;
    case 0b1011011: return "5"; break;
    case 0b1011111: return "6"; break;
    case 0b1110000: return "7"; break;
    case 0b1111111: return "8"; break;
    case 0b1110011: return "9"; break;
    case 0b1110111: return "A"; break;
      // case 0b0011111: return "B";  break;
    case 0b1001110: return "C"; break;
      // case 0b0111101: return "D";  break;
    case 0b1001111: return "E"; break;
      // case 0b1000111: return "F";  break;
    case 0b1011110: return "G"; break;
    case 0b0110111: return "H"; break;
      // case 0b0000110: return "I";  break;
    case 0b0111100: return "J"; break;
      // case 0b1010111: return "K";  break;
    case 0b0001110: return "L"; break;
    case 0b1010100: return "M"; break;
    case 0b1110110: return "N"; break;
      // case 0b1111110: return "O";  break;
    case 0b1100111: return "P"; break;
    case 0b1101011: return "Q"; break;
    case 0b1100110: return "R"; break;
      // case 0b1011011: return "S";  break;
      // case 0b0001111: return "T";  break;
    case 0b0111110: return "U"; break;
      // case 0b0111110: return "V";  break;
    case 0b0101010: return "W"; break;
      // case 0b0110111: return "X";  break;
      // case 0b0111011: return "Y";  break;
      // case 0b1101101: return "Z";  break;
    case 0b1111101: return "a"; break;
    case 0b0011111: return "b"; break;
    case 0b0001101: return "c"; break;
    case 0b0111101: return "d"; break;
    case 0b1101111: return "e"; break;
    case 0b1000111: return "f"; break;
      // case 0b1111011: return "g";  break;
    case 0b0010111: return "h"; break;
    case 0b0000100: return "i"; break;
    case 0b0000001: return "j"; break;
    case 0b1010111: return "k"; break;
    case 0b0000110: return "l"; break;
    case 0b0010100: return "m"; break;
    case 0b0010101: return "n"; break;
    case 0b0011101: return "o"; break;
      //case 0b1100111: return "p";  break;
      //  case 0b1110011: return "q";  break;
    case 0b0000101: return "r"; break;
      // case 0b1011011: return "s";  break;
    case 0b0001111: return "t"; break;
    case 0b0011100: return "u"; break;
      // case 0b0011100: return "v";  break;
      // case 0b0010100: return "w";  break;
      // case 0b0110111: return "x";  break;
    case 0b0111011: return "y"; break;
    default: return ""; break;  // Error condition, displays vertical bars
  }
}
