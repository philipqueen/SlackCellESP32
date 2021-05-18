/*

Based on Slackcell code by Markus Rampp

Designed for use with ESP32 using SSD1306_128x64 OLED Display
Tested with and recommended for Heltec WifiKit32 controller

Future improvements:
  1) Separate display values to core 0 to speed up read speed
  2) Improve general read speed from 65 to 80 Hz
  3) Implement BLE connection for app integration

*/

#include "HX711.h" //library for loadcell amplifier
//#include "BT.h"
#include <U8g2lib.h> //library for OLED
//#include <Wire.h>

// libraries for SD card
#include "FS.h"
#include "SD.h"
#include <spi.h> //also needed for some OLEDs


const long baud = 115200;

// HX711 circuit wiring
const int LOADCELL_SCK_PIN = 26;
const int LOADCELL_DOUT_PIN = 25;
const long LOADCELL_OFFSET = 2900;
const long LOADCELL_DIVIDER_N = -217;
const long LOADCELL_DIVIDER_kg = LOADCELL_DIVIDER_N * 9.81;
const long LOADCELL_DIVIDER_lb = LOADCELL_DIVIDER_N * 4.448;

// SD Card circuit wiring
#define SD_CS 2

HX711 loadcell; //setup HX711 object
U8G2_SSD1306_128X64_NONAME_F_HW_I2C   u8g2(U8G2_R0, /*reset= */ 16, /*clock=*/ 15, /*data=*/ 4); //setup display connection


unsigned long timestamp = 0;
long maxForce = 0;
long force = -1;
long prevForce = -100;
const int Switch = 22;
//bool recording = false;
//char prev_cmd = '0';

String sdMessage;
int readingID = 0;

unsigned long timeNow = 0;


void setup() {
  Serial.begin(baud);
  Serial.println("Welcome to SlackCell!");
  Serial.print("Sketch:   ");   Serial.println(__FILE__);
  Serial.print("Uploaded: ");   Serial.println(__DATE__);

  // This statement will declare pin 15 as digital input 
  pinMode(Switch, INPUT);

  u8g2.setBusClock(1000000);
  u8g2.begin();
  u8g2.setPowerSave(0);
  u8g2.setFont(u8g2_font_inb21_mf);
  //u8g2.setFontRefHeightExtendedText();
  //u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  //u8g2.setFontDirection(0);

  u8g2.clearBuffer();
  u8g2.drawStr(0, 0, "SLACK");
  u8g2.drawStr(0, 36, "CELL");
  u8g2.sendBuffer();

  loadcell.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  loadcell.set_offset(LOADCELL_OFFSET);
  loadcell.set_scale(LOADCELL_DIVIDER_N);
  force = loadcell.get_units(30);
  if (force < 30) {
    loadcell.tare();
    force = 0;
  }

  delay(2000);
  u8g2.clearBuffer();

  // Initialize SD card
  SD.begin(SD_CS);  
  if(!SD.begin(SD_CS)) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }
  Serial.println("Initializing SD card...");
  if (!SD.begin(SD_CS)) {
    Serial.println("ERROR - SD card initialization failed!");
    return;    // init failed
  }

  // If the data.txt file doesn't exist
  // Create a file on the SD card and write the data labels
  File file = SD.open("/data.txt");
  if(!file) {
    Serial.println("File doens't exist");
    Serial.println("Creating file...");
    writeFile(SD, "/data.txt", "Reading ID, Time (ms), Force (N) \r\n"); //move out of if statement so it separates logging sessions?
  }
  else {
    Serial.println("File already exists");  
  }
  file.close();
}

void loop() {
  if (loadcell.is_ready()) {
    Serial.print("Reading: ");
    force = loadcell.get_units(1);
    timeNow = millis(); //milliseconds since startup
    Serial.print(abs(force), 1); //prints first sigfig of force
    Serial.print(" N"); //change depending on divider used
    Serial.println();
    int Switch_state = digitalRead(Switch);
    if ((force != prevForce) && (Switch_state == LOW)) {
        prevForce = force;
        maxForce = max(abs(force), abs(maxForce));
        // display updates only value at a time to increase speed, privileges maxForce
        if (maxForce == abs(force)) {
          displayForce(maxForce, 36);
        }
        else {
          displayForce(force, 0);
        }
      }
    writeSD(readingID, timeNow, force);
    readingID += 1;
  }
  
}

void displayForce(long force, uint8_t line) {
  // Maximum number of letters the screen can display in one row
  uint8_t maxLength = 6;
  // Long range is +-2,147,483,648
  char bufferF[12] = {};
  ltoa(force, bufferF, 10);
  // align right by padding with spaces
  char bufferLCD[maxLength + 1] = {' ', ' ', ' ', ' ', ' ', ' ', '\0'};
  for (int i = 0; i < strlen(bufferF); i++) {
    bufferLCD[maxLength - strlen(bufferF) + i] = bufferF[i];
  }
  u8g2.drawStr(0, line, bufferLCD);
  u8g2.sendBuffer();
}

void writeSD(int readingID, long timeNow, long force) {
  sdMessage = String(readingID) + "," + String(timeNow) + "," + String(force) + "\r\n";
  appendFile(SD, "/data.txt", sdMessage.c_str());
}

// Write to the SD card (DON'T MODIFY THIS FUNCTION)
void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

// Append data to the SD card (DON'T MODIFY THIS FUNCTION)
void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}
