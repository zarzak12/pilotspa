#ifndef Balboa_GS_Interface_h
#define Balboa_GS_Interface_h

#include <Arduino.h>


const byte displayDataBufferSize             = 75;     // Size of display data buffer
const byte displayDataBits                  = 71;     // 0-70 bits length of display data within a cycle
const byte buttonDataBits                   = 3;      // 0-3 bits length of button data within a cycle
const byte totalDataBits                    = 75;     // 0-73 total number of pulses within a cycle
const unsigned int durationNewCycle         = 12000;   // How many microsecounds to detect new cycle if no interrupt occurs -- old 5000
const unsigned long buttonPressTimerMillis  = 500;  // Timer in milliseconds between update temperature button presses 
const String stdModeProg = "Std";
const String ecoModeProg = "Ecn";
const String slpModeProg = "SLP";

class BalboaInterface {

  public:
  
  BalboaInterface(byte setClockPin, byte setReadPin, byte setWritePin);
  
  // Interface control
  void begin();                                   // Initializes the stream output to Serial by default
  bool loop();                                    // Returns true if valid data is available
  void stop();                                    // Disables the clock hardware interrupt 
  void resetStatus();                             // Resets the state of all status components as changed for sketches to get the current status  
  void updateTemperature(float Temperature);      // Function to set the water temperature  
  void updateMode(String Mode);                   // Function to set the mode  
  void updateTime(int heureSet, int minuteSet);         // Function to set the time 
  void getTime();         // Function to set the time 
  void getFilterTime();
  void setFilter1();
  void setFilter2();
  void calculImpultionfilter1_1();
  void calculImpultionfilter1_2();
  int convertirHeureEnMinutes(int heureSpa, int minuteSpa);
  void updateTimeFilter(int heureSet, int minuteSet, int mode);

  // Status tracking
  bool displayBit0;
  float waterTemperature;           // Water temperatur 
  float setTemperature;             // The wanted set temperature   
  String LCD_display;               // The text shown on display 
  bool displayButton;               // Temp up/down button pressed
  bool displayBit29;                // Still unknown functionality, if at all used!
  bool displayBit30;                // Still unknown functionality, if at all used!
  bool displayBit31;                // Standard Mode activated or not
  bool displayBit32;                // Still unknown functionality, if at all used!
  bool displayBit33;                // Still unknown functionality, if at all used!  
  bool displayBit34;                // Still unknown functionality, if at all used!
  bool displayBit35;               // Heater running or not
  bool displayFilter1;                // Pump 1 running or not 
  bool displayFilter2;                // Pump 2 running or not
  bool displayBit38;                // Hot tube lights activated or not
  bool displayHeater; 
  bool displayBit40; 
  bool displayBit41; 
  bool displayBlower;
  bool displaySetFilter; 
  bool displayFiltration; 
  bool displayBit45; 
  bool displayBit46; 
  bool displayLight; 
  bool displayPump1; 
  bool displayPump2; 
  bool displayStop; 
  bool displayBit51; 
  bool displayBit52; 
  bool displayBit53; 
  bool displayBit54; 
  bool displayTime; 
  bool displaySetTime; 
  bool displayBit57; 
  bool displayStart;
  bool displayStandardMode;
  bool displayEcoMode;
  bool displayPM;
  bool displayBit62;
  bool displayAM;
  bool displayBit64;
  bool displayBit65;
  bool displayBit66;
  bool displayBit67;
  bool displayBit68;
  bool displayBit69;
  bool displayBit70;
  bool displayBit71;
  static bool displayDataBufferOverflow;
  String displayMode = "Inconnu";
  int differenceMinutes=0;
  int differenceMinutesUFT=0;
  bool timeDifference=false;
  int impulsionsFilter1_1=0;
  int impulsionsFilter1_2=0;
  int impulsionsFilter2_1=0;
  int impulsionsFilter2_2=0;
  
  // Write button data to control unit  
  static bool writeDisplayData;               // If something should be written to button data line  
  static bool writeMode;
  static bool writeTempUp;
  static bool writeTempDown;
  static bool writeLight;
  static bool writePump1;
  static bool writePump2;   
  static bool writePump3;
  static bool writeBlower;
  static bool writeTime;

  static volatile byte displayDataBuffer[displayDataBufferSize];   // Array of display data measurements 
  static volatile bool displayDataBufferReady;                 // Is buffer available to be decoded

  int heureLue=12;
  int minuteLue=0;
  int heureFilter1Deb=8;
  int heureFilter1Fin=10;
  int heureFilter2Deb=8;
  int heureFilter2Fin=10;
  int minuteFilter1Deb=0;
  int minuteFilter1Fin=0;
  int minuteFilter2Deb=0;
  int minuteFilter2Fin=0;
  
  private:
  
  static void clockPinInterrupt();
  void decodeDisplayData();
  String lockup_LCD_character(int LCD_character);
  int LCD_segment_1;
  int LCD_segment_2;
  int LCD_segment_3;
  int LCD_segment_4;
  String LCD_display_1;
  String LCD_display_2;
  String LCD_display_3;
  String LCD_display_4;  
  //static byte displayDataBuffer[displayDataBufferSize];   // Array of display data measurements 
  static unsigned long clockInterruptTime;
  static int clockBitCounter;                       // Counter of pulses within a cycle
  static byte dataIndex;                  
  static byte clockPin;
  static byte displayPin;
  static byte buttonPin;

  int updateTempDirection;
  int updateTempButtonPresses;
  unsigned long buttonPressTimerPrevMillis; 

  int updateModeButtonPresses=0;
  int callTimeButtonPresses=0;
  
  int updateTimeFTButtonPresses=0;
  int updateModeFTButtonPresses=0;

  int updateTimeFT1ButtonPresses=0;
  int updateModeFT1ButtonPresses=0;

  int updateTimeFT2ButtonPresses=0;
  int updateModeFT2ButtonPresses=0;

  int updateTimeUFTButtonPresses=0;
  int updateModeUFTButtonPresses=0;
  
  int updateHeureDirection;
  int updateHeureDirectionUFT;

  bool heureOK = true;
  bool modePush = false;
  bool timePush = false;
  bool modeAM = false;
  bool modePM = false;
  bool callTime = false;
  bool callTimeFilter = false;
  bool updateTimeFilterBool = false;
  bool filter1 = false;
  bool filter2 = false;

  bool traitementEnCours = false;

};

  
#endif  // Balboa_GS_Interface_h
