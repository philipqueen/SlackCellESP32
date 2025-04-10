/*

Based on Slackcell code by Markus Rampp (copyright 2020)

Designed for use with ESP32 using SSD1306_128x64 OLED Display
Tested with and recommended for Heltec WifiKit32 V2 or V3 controller

*/

#include <Arduino.h>

// libraries for SD card
#include "FS.h"
#include "SD.h"

#include "pins.h"
#include "display.h"

//moved below pins to capture USE_HX71708 declaration
#ifdef USE_HX71708
#include <HX71708.h> // alpha fork for HX71708
#else
#include "HX711.h" //library for loadcell amplifier
#endif

#define CSV_NAME "/slackcell.txt" // TODO: not sure if these should live in another file
#define CSV_HEADER "Reading ID, Time (µs), Force (N) \r\n"
#define SD_MESSAGE_LENGTH 60
#define SD_START_DELAY 2000
#define SD_BATCH_SIZE 16

#define TARE_AVERAGE_TIME 30
#define MAX_TARE_VALUE 30

//Function prototypes (needed for Platform IO and every other normal c++ file, its just the Arduino IDE uses magic to get rid of them)
void init_sd();
void writeFile(fs::FS &fs, const char * path, const char * message);
void appendFile(fs::FS &fs, const char * path, const char * message);
void writeSD(int readingID, long timeNow, long force);
void appendMeasurement(void * parameter);
void Display(void * parameter);

#ifdef USE_VEXT
void VextON(void);
void VextOFF(void);
#endif

#ifdef USE_VSPI
SPIClass spiVspi(VSPI);
#endif
  
#ifdef USE_BUTTON
#include "OneButton.h"
OneButton display_active_btn(BUTTON_PIN, true);
void toggleSwitchState();
void resetMaxForce();
#endif

const long baud = 115200;

//const long LOADCELL_OFFSET = 2330;
//const float LOADCELL_DIVIDER_N = -232;

const long LOADCELL_OFFSET =18911;
const float LOADCELL_DIVIDER_N = 1491.765;

const float LOADCELL_DIVIDER_kg = LOADCELL_DIVIDER_N * 9.81;
const float LOADCELL_DIVIDER_lb = LOADCELL_DIVIDER_N * 4.448;


#ifdef USE_HX71708
HX71708 loadcell; //setup HX711 object
#else
HX711 loadcell; //setup HX711 object
#endif


TaskHandle_t DisplayTask;
TaskHandle_t appendMeasurementTask;
QueueHandle_t queue;
QueueHandle_t sdQueue;

File dataFile;

unsigned long timestamp = 0;
long maxForce = 0;
long force = -1;
long reading = -1;
long avg_reading = 0;
long prevForce = -100;
long prevMaxForce = -100;

// TODO: should be selectable with buttons, maybe a small menu?
bool recording = true;
// Set in init_sd, signals readiness to use the data.txt file on the sd.
// This allows the program to run without an sd card connected, without trying to write to it all the time.
bool sd_ready = false;

String sdMessage;
int readingID = 0;

unsigned long timeNow = 0;

bool Switch_state = false;

void setup() {
  Serial.begin(baud);
  Serial.println("Welcome to SlackCell!");
  Serial.print("Sketch:   ");   Serial.println(__FILE__);
  Serial.print("Uploaded: ");   Serial.println(__DATE__);

#ifdef USE_VEXT
  //turn on external devices
  VextON();
#endif //USE_VEXT

#ifdef USE_SWITCH
  // Setting up the Switch
  pinMode(SWITCH_PIN, SWITCH_MODE);
#endif
#ifdef USE_BUTTON
  // Setting up the Button
  display_active_btn.attachClick(toggleSwitchState);
  display_active_btn.attachLongPressStart(resetMaxForce);
#endif

  displayInit();

  loadcell.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  loadcell.set_offset(LOADCELL_OFFSET);
  loadcell.set_scale(LOADCELL_DIVIDER_N);
  force = loadcell.get_units(TARE_AVERAGE_TIME);
  if (force < MAX_TARE_VALUE) {
    loadcell.tare();
    force = 0;
  }

  queue = xQueueCreate(2, sizeof(long));

  if(queue == NULL){
    Serial.println("Error creating the queue");
  }

  sdQueue = xQueueCreate(64, sizeof(String));
  if (!sdQueue) {
    Serial.println("Error creating SD queue");
  }

  xTaskCreatePinnedToCore(
    Display, /* Function to implement the task */
    "DisplayTask", /* Name of the task */
    10000,  /* Stack size in words */
    NULL,  /* Task input parameter */
    0,  /* Priority of the task */
    &DisplayTask,  /* Task handle. */
    0); /* Core where the task should run */


  

  //This might seem like a unnecessary start up delay, just to see "Slack Cell" longer...
  //but it also stabilizes the signal levels to not have multiple kilos of maxForce just from booting up...
  //and gives the SD card time to initialize
  delay(SD_START_DELAY);
  init_sd();


  xTaskCreatePinnedToCore(
    appendMeasurement, "appendMeasurementTask", 
    8192, 
    NULL, 
    1, 
    &appendMeasurementTask, 
    1);


  sdMessage.reserve(SD_MESSAGE_LENGTH);


  displayClearBuffer();
}


// Connects to the sd card via spi and makes sure the file used for data logging exists.
// If successful sd_ready is set to true
void init_sd(){
  if(sd_ready)
    //already initialized, skipping
    return;

  sdMessage.reserve(SD_MESSAGE_LENGTH);

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
  // Create a file on the SD card
  File file = SD.open("/data.txt");
  if(!file) {
    Serial.println("File doens't exist");
    Serial.println("Creating file...");
  }
  else {
    Serial.println("File already exists");
  }
  //appendFile(SD, CSV_NAME, CSV_HEADER); // We always append the header to demarcate different sessions
  file.close();

  sd_ready = true;
  // TODO: display a striked out SD in one corner if not available. But don't do it here, because the display is cleared afterwards
}

void loop() {
  #ifdef USE_SWITCH
      Switch_state = (digitalRead(SWITCH_PIN) == HIGH);
  #elif defined(USE_BUTTON)
      display_active_btn.tick();
  #endif

  if (loadcell.is_ready()) {
    reading = std::lround(loadcell.get_units(1));
    timeNow = micros(); //milliseconds since startup
    maxForce = max(reading, maxForce);

    if(Switch_state == false){
#if READING_AVG_TIMES == 1
      xQueueSend(queue, &reading, 0);
#else
      //The display of the Heltec V3 for example is so slow, that it drops 80% of the values of the display queue, so we might as well average them.
      avg_reading += reading;
      //This modulo construct calls the first branch every READING_AVG_TIMES-th time and sends the force to the display.
      if(readingID % READING_AVG_TIMES == 0){
        //the first avg_reading after boot will be wrong, but it will only be seen for a few ms once, and will never be written to sd. So we just don't care.
        avg_reading = std::lround(avg_reading / float(READING_AVG_TIMES));
        //If the queue is full, the reading just get's discarded for the display, so the mcu can continue writing to the sd.
        //Keeping the sampling rate for sd is more import than displaying
        xQueueSend(queue, &avg_reading, 0);
        avg_reading = 0;
      }
#endif

    }
    Serial.printf("Reading: %ld N\n", abs(reading));
    
    if (sd_ready && recording) {
      writeSD(readingID, timeNow, reading);
    }
    readingID++;
  }
}

void Display(void * parameter) {
  for(;;){
    xQueueReceive(queue, &force, portMAX_DELAY);
    Serial.println(force);
    if (force != prevForce) {
      prevForce = force;
      displayForce(force);
    }
    if(maxForce != prevMaxForce) {
      prevMaxForce = maxForce;
      displayMaxForce(maxForce);
    }
  }
}

void appendMeasurement(void * parameter) {
  String* msg;
  String buffer = "";
  int count = 0;

  dataFile = SD.open(CSV_NAME, FILE_APPEND);
  if (!dataFile) {
    Serial.println("Failed to open file in writer task.");
    vTaskDelete(NULL);
    return;
  }

  dataFile.println(CSV_HEADER);
  dataFile.flush();

  while (true) {
    if (xQueueReceive(sdQueue, &msg, portMAX_DELAY)) {
      buffer += *msg + "\n";
      delete msg;  // free the memory after copying the data
      count++;

      if (count >= SD_BATCH_SIZE) {
        dataFile.print(buffer);
        dataFile.flush();
        buffer = "";
        count = 0;
      }
    }
  }
}


void writeSD(int readingID, long timeNow, long force) {
  String* msg = new String(String(readingID) + "," + timeNow + "," + force);
  xQueueSend(sdQueue, &msg, 0); // send the pointer
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
  //Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)) {
    //Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

#ifdef USE_BUTTON
void toggleSwitchState(){
  Switch_state = !Switch_state;
}

void resetMaxForce(){
  maxForce = 0;
}
#endif

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