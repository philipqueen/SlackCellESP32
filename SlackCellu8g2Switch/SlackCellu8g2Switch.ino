/*

Based on Slackcell code by Markus Rampp (copyright 2020)

Designed for use with ESP32 using SSD1306_128x64 OLED Display
Tested with and recommended for Heltec WifiKit32 controller

*/

#include "HX711.h"
#include <TFT_eSPI.h>
#include <SPI.h>

// libraries for SD card
#include "FS.h"
#include "SD.h"

#define SD_CS 2
#define SD_MOSI 26
#define SD_MISO 27
#define SD_SCK 25


const long baud = 115200;

const int LOADCELL_SCK_PIN = 33;
const int LOADCELL_DOUT_PIN = 32;
const long LOADCELL_OFFSET = 2330;
const long LOADCELL_DIVIDER_N = -232;
const long LOADCELL_DIVIDER_kg = LOADCELL_DIVIDER_N * 9.81;
const long LOADCELL_DIVIDER_lb = LOADCELL_DIVIDER_N * 4.448;

HX711 loadcell; //setup HX711 object
TFT_eSPI tft = TFT_eSPI();
SPIClass spiVspi(VSPI);

unsigned long timestamp = 0;
long maxForce = 0;
long force = -1;
long prevForce = -100;
const int Switch = 22;
String sdMessage;
int readingID = 0;
unsigned long timeNow = 0;

void setup() {
  Serial.begin(baud);
  Serial.println("Welcome to SlackCell!");
  Serial.print("Sketch:   ");   Serial.println(__FILE__);
  Serial.print("Uploaded: ");   Serial.println(__DATE__);

  tft.init();
  tft.setRotation(1); // Set the screen to horizontal layout
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(3);

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(10, 20);
  tft.printf("SLACK");
  tft.setCursor(10, 60);
  tft.printf("CELL");

  loadcell.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  loadcell.set_offset(LOADCELL_OFFSET);
  loadcell.set_scale(LOADCELL_DIVIDER_N);
  force = loadcell.get_units(30);
  if (force < 30) {
    loadcell.tare();
    force = 0;
  }

  spiVspi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

  if (!SD.begin(SD_CS, spiVspi)) {
    Serial.println("Card Mount Failed");
    return;
  }

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  Serial.println("Initializing SD card...");
  File file = SD.open("/data.txt");
  if (!file) {
    Serial.println("File doesn't exist");
    Serial.println("Creating file...");
    writeFile(SD, "/data.txt", "Reading ID, Time (ms), Force (N) \r\n");
  } else {
    Serial.println("File already exists");
  }
  file.close();

  delay(2000);
}

void loop() {
  if (loadcell.is_ready()) {
    Serial.print("Reading: ");
    force = loadcell.get_units(1);
    timeNow = millis(); //milliseconds since startup
    Serial.print(abs(force), 1); //prints first sigfig of force
    Serial.print(" N"); //change depending on divider used
    Serial.println();
    if ((force != prevForce)) {
        prevForce = force;
        maxForce = max(abs(force), abs(maxForce));

        tft.fillRect(10, 20, 180, 30, TFT_BLACK);
        tft.setCursor(10, 20); // Set x, y position in pixels
        tft.setTextColor(TFT_BLUE, TFT_BLACK);
        tft.printf("live: %ld", force);

        tft.fillRect(10, 60, 180, 30, TFT_BLACK); // need to make this extend further right
        tft.setCursor(10, 60);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.printf("peak: %ld", maxForce);
    }
    writeSD(readingID, timeNow, force);
    readingID += 1;

    delay(100);
  } 
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

