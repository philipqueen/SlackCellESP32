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

#define CSV_NAME "/slackcell.txt"

#define N_TO_KG 9.81
#define N_TO_LB 4.448


const long baud = 115200;

const int LOADCELL_SCK_PIN = 33;
const int LOADCELL_DOUT_PIN = 32;
const long LOADCELL_OFFSET = 2330;
const long LOADCELL_DIVIDER_N = -232;
const long LOADCELL_DIVIDER_kg = LOADCELL_DIVIDER_N * N_TO_KG;
const long LOADCELL_DIVIDER_lb = LOADCELL_DIVIDER_N * N_TO_LB;

HX711 loadcell;
TFT_eSPI tft = TFT_eSPI();
SPIClass spiVspi(VSPI);

unsigned long timestamp = 0;
long maxForce = 0;
long force = -1;
long prevForce = -100;
const int Switch = 22;
int readingID = 0;
unsigned long timeNow = 0;
String sdMessage;


void setup() {
  Serial.begin(baud);
  Serial.println("Welcome to SlackCell!");
  Serial.print("Sketch:   ");   Serial.println(__FILE__);
  Serial.print("Uploaded: ");   Serial.println(__DATE__);

  tft.init();
  tft.setRotation(1); // horizontal layout
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(3);

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

  sdMessage.reserve(60);

  spiVspi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);  // prevent SD from interfering with screen HSPI connection

  delay(2000);

  if (!SD.begin(SD_CS, spiVspi, 4000000)) {
    Serial.println("Card Mount Failed");
    return;
  }

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  Serial.println("Initializing SD card...");
  File file = SD.open(CSV_NAME);
  if (!file) {
    Serial.println("File doesn't exist");
    Serial.println("Creating file...");
  } else {
    Serial.println("File already exists");
  }
  writeFile(SD, CSV_NAME, "Reading ID, Time (ms), Force (N) \n");
  file.close();
}

void loop() {
  if (loadcell.is_ready()) {
    force = loadcell.get_units(1);
    timeNow = millis(); //milliseconds since startup
    Serial.printf("Reading: %ld N\n", abs(force));
    if ((force != prevForce)) {
        prevForce = force;
        maxForce = max(force, maxForce);

        tft.fillRect(10, 20, 180, 30, TFT_BLACK);
        tft.setCursor(10, 20); // x, y position
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
  sdMessage = "";
  sdMessage += readingID;
  sdMessage += ",";
  sdMessage += timeNow;
  sdMessage += ",";
  sdMessage += force;
  sdMessage += "\n";
  appendFile(SD, CSV_NAME, sdMessage.c_str());
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