/*

Based on Slackcell code by Markus Rampp

*/

#include "HX711.h" //library for loadcell amplifier
//#include "BT.h"
#include <U8x8lib.h> //library for OLED

// libraries for SD card
#include "FS.h"
#include "SD.h"
#include <spi.h> //also needed for some OLEDs


const long baud = 38400;

// HX711 circuit wiring
const int LOADCELL_SCK_PIN = 26;
const int LOADCELL_DOUT_PIN = 25;
const long LOADCELL_OFFSET = 10900;
const long LOADCELL_DIVIDER_N = -445;
const long LOADCELL_DIVIDER_kg = LOADCELL_DIVIDER_N * 9.81;
const long LOADCELL_DIVIDER_lb = LOADCELL_DIVIDER_N * 4.448;

// SD Card circuit wiring
#define SD_CS 2

HX711 loadcell; //setup HX711 object
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16); //setup display connection


unsigned long timestamp = 0;
long maxForce = 0;
long force = 0;
long prevForce = -100;
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

  u8x8.begin();
  u8x8.setPowerSave(0);
  // fontlist: https://github.com/olikraus/u8g2/wiki/fntlist8x8
  // n: numeric, f: with chars
  u8x8.setFont(u8x8_font_profont29_2x3_n );
  //u8x8.setFont(u8x8_font_inb21_2x4_f );

  //u8x8.drawString(0,0,"Slack");
  //u8x8.drawString(0,4,"Cell");
  u8x8.drawString(0, 0, "11235813");
  u8x8.drawString(0, 4, "21345589");

  loadcell.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  loadcell.set_offset(LOADCELL_OFFSET);
  loadcell.set_scale(LOADCELL_DIVIDER_N);
  force = loadcell.get_units(30);
  if (force < 30) {
    loadcell.tare();
    force = 0;
  }

  // delay(1000);
  u8x8.clear();

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
    if (force != prevForce) {
        //display current force
        displayForce(force, 0);
        prevForce = force;
        maxForce = max(abs(force), abs(maxForce));
        // update display with maxForce only if it changes to reduce flickering
        if (maxForce == abs(force))
          displayForce(maxForce, 4);
      }
    writeSD(readingID, timeNow, force);
    readingID += 1;
  }
}

void displayForce(long force, uint8_t line) {
  // Maximum number of letters the screen can display in one row
  uint8_t maxLength = 8;
  // Long range is +-2,147,483,648
  char bufferF[12] = {};
  ltoa(force, bufferF, 10);
  // align right by padding with spaces
  char bufferLCD[maxLength + 1] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\0'};
  for (int i = 0; i < strlen(bufferF); i++) {
    bufferLCD[maxLength - strlen(bufferF) + i] = bufferF[i];
  }
  u8x8.drawString(0, line, bufferLCD);
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
