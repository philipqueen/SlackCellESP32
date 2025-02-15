/*

Based on Slackcell code by Markus Rampp (copyright 2020)

Designed for use with ESP32 using SSD1306_128x64 OLED Display
Tested with and recommended for Heltec WifiKit32 V2 or V3 controller

*/

#include <Arduino.h>

#include "HX711.h" //library for loadcell amplifier
#include "U8g2lib.h" //library for OLED

// libraries for SD card
#include "FS.h"
#include "SD.h"

#include "pins.h"

//Function prototypes (needed for Platform IO and every other normal c++ file, its just the Arduino IDE uses magic to get rid of them)
void init_sd();
void displayForce(long force, uint8_t line);
void writeSD(int readingID, long timeNow, long force);
void writeFile(fs::FS &fs, const char * path, const char * message);
void appendFile(fs::FS &fs, const char * path, const char * message);

#ifdef USE_VEXT
void VextON(void);
void VextOFF(void);
#endif

const long baud = 115200;

const long LOADCELL_OFFSET = 2330;
const float LOADCELL_DIVIDER_N = -232;
const float LOADCELL_DIVIDER_kg = LOADCELL_DIVIDER_N * 9.81;
const float LOADCELL_DIVIDER_lb = LOADCELL_DIVIDER_N * 4.448;

HX711 loadcell; //setup HX711 object
U8G2_SSD1306_128X64_NONAME_F_HW_I2C   u8g2(U8G2_R2, OLED_RESET_PIN, OLED_CLOCK_PIN, OLED_DATA_PIN); //setup display connection


unsigned long timestamp = 0;
long maxForce = 0;
long force = -1;
long prevForce = -100;

// TODO: should be selectable with buttons, maybe a small menu?
bool recording = true;
// Set in init_sd, signals readiness to use the data.txt file on the sd.
// This allows the program to run without an sd card connected, without trying to write to it all the time.
bool sd_ready = false;

String sdMessage;
int readingID = 0;

unsigned long timeNow = 0;


void setup() {
  Serial.begin(baud);
  Serial.println("Welcome to SlackCell!");
  Serial.print("Sketch:   ");   Serial.println(__FILE__);
  Serial.print("Uploaded: ");   Serial.println(__DATE__);

#ifdef USE_VEXT
  //turn on external defices
  VextON();
#endif //USE_VEXT

  // Setting up the Switch
  pinMode(SWITCH_PIN, SWITCH_MODE);

  u8g2.setBusClock(1000000);
  u8g2.begin();
  u8g2.setPowerSave(0);
  u8g2.setFont(u8g2_font_inb21_mf);
  u8g2.setFontPosTop();


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

  init_sd();

  //This might seem like a unnecessary start up delay, just to see "Slack Cell" longer...
  //but it also stabilizes the signal levels to not have multiple kilos of maxForce just from booting up.
  delay(2000);
  u8g2.clearBuffer();

}


// Connects to the sd card via spi and makes sure the file used for data logging exists.
// If successful sd_ready is set to true
void init_sd(){
  if(sd_ready)
    //already initialized, skipping
    return;

  #ifdef CUSTOM_SPI_PINS
  SPI.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
  #endif

  // Initialize SD card
  if(!SD.begin(SD_CS_PIN)) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }
  Serial.println("Initializing SD card...");
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("ERROR - SD card initialization failed!");
    return;
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

  sd_ready = true;
  // TODO: display a striked out SD in one corner if not available. But don't do it here, because the display is cleared afterwards
}

void loop() {
  if (loadcell.is_ready()) {
    Serial.print("Reading: ");
    force = loadcell.get_units(1);
    timeNow = millis(); //milliseconds since startup
    Serial.print(abs(force), 1); //prints first sigfig of force
    Serial.print(" N"); //change depending on divider used
    Serial.println();
    int Switch_state = digitalRead(SWITCH_PIN);
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

    if(sd_ready && recording){
      writeSD(readingID++, timeNow, force);
    }
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

#ifdef USE_VEXT
//Turn external power supply on
void VextON(void)
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, LOW);
  
}

//Turn external power supply off
void VextOFF(void) //Vext default OFF
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, HIGH);
}
#endif //USE_VEXT