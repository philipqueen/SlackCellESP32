// Use this sketch to calibrate your loadcell.
// You need your arduino connected to a HX711 module and a loadcell
// and a computer to read the serial output of your arduino
//
// First use it with an unloaded loadcell. LOADCELL_OFFSET should roughly equal 
// the Raw value in output. 
//
// Next measure a calibration weight that is roughly in the magnitude of your 
// usecase (for slacklines you can use your bodyweight). Don't forget to include
// all the attachments for the loadcell
//
// With the LOADCELL_OFFSET configured correctly you can now get your LOADCELL_DIVIDER right. 
// Attach the calibration weight and change the LOADCELL_DIVIDER to the recommendedDivider
// in the output

#include <stdio.h>
#include "HX711.h"

//      D2  ||  SCK   HX711
//      D4  ||  DOUT  HX711

//Change to the board you want to use, and check the wiring defined below
//#define BOARD_HELTEC_V2
// #define BOARD_HELTEC_V3
#define BOARD_TTGO_DISPLAY

#if defined(BOARD_HELTEC_V2)
// HX711 circuit wiring
const uint8_t LOADCELL_DOUT_PIN = 25;
const uint8_t LOADCELL_SCK_PIN = 26;

#elif defined(BOARD_HELTEC_V3)
// HX711 circuit wiring
const uint8_t LOADCELL_DOUT_PIN = 41;
const uint8_t LOADCELL_SCK_PIN = 40;

#elif defined(BOARD_TTGO_DISPLAY)
// HX711 circuit wiring
const uint8_t LOADCELL_DOUT_PIN = 32;
const uint8_t LOADCELL_SCK_PIN = 33;

#endif

const long LOADCELL_OFFSET = 2900;

// divider to get the force in the right units. I'm using logical units [N]
const float LOADCELL_DIVIDER_N = 223.46;
const float LOADCELL_DIVIDER_kg = LOADCELL_DIVIDER_N * 9.81;
const float LOADCELL_DIVIDER_lb = LOADCELL_DIVIDER_N * 4.448;


// Hang yourself on the scale and weight you with and everything 
// you need for hanging you there on a trustworthy scale
const double calibrationWeight = 72.575;

const double calibrationForce = calibrationWeight * 9.81;

long maxForce = 0;
long force = 0;
long prevForce = 0;
HX711 loadcell;

void setup() {
  Serial.begin(115200);
  Serial.println("Welcome to SlackCell Calibrating!");
  Serial.print("Sketch:   ");   Serial.println(__FILE__);
  Serial.print("Uploaded: ");   Serial.println(__DATE__);

  loadcell.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  loadcell.set_offset(LOADCELL_OFFSET);
  loadcell.set_scale(LOADCELL_DIVIDER_N);
  delay(1000);
}

void loop() {
  if (loadcell.is_ready()) {
    long value = loadcell.read_average(30);
    long valueWithOffset = value - LOADCELL_OFFSET;
    long withDivider =  (value - LOADCELL_OFFSET)/LOADCELL_DIVIDER_N;
    //LOADCELL_OFFSET should roughly equal Raw -> withOffset should be 0 without any force on the loadcell
    //withDivider should equal calibration weight
    Serial.println("     Raw  withOffset  withDivider  recommendedDivider");
    char string [60];
    sprintf(string,"%8ld  %10ld  %11ld", value, value - LOADCELL_OFFSET, withDivider);
    Serial.print(string);
    Serial.print("         ");    
    Serial.println((value - LOADCELL_OFFSET)/ calibrationForce);
  }  
}
