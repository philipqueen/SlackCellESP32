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

#include <ADS1220_WE.h>
#include <SPI.h>

#define CSV_NAME "/slackcell.bin" // TODO: not sure if these should live in another file
#define CSV_HEADER "Reading ID, Time (µs), Force (N) \r\n"
#define SD_MESSAGE_LENGTH 60
#define SD_START_DELAY 2000
#define SD_BATCH_SIZE 256

#define TARE_AVERAGE_TIME 30
#define MAX_TARE_VALUE 30

struct LogEntry {
  int32_t logID;
  uint32_t logMicros;
  float logForce;
};

int droppedlogs=0;

//Function prototypes (needed for Platform IO and every other normal c++ file, its just the Arduino IDE uses magic to get rid of them)
void init_sd();
void writeFile(fs::FS &fs, const char * path, const char * message);
void appendFile(fs::FS &fs, const char * path, const char * message);
void appendMeasurement(void * parameter);
void Display(void * parameter);
void getMeasurement(void * parameter);
void setSampleRate(bool turbo, uint8_t rateLevel);

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

SPIClass spiADS1220(HSPI);  // Create the SPI instance
ADS1220_WE loadcell = ADS1220_WE(&spiADS1220, ADS1220_CS_PIN, ADS1220_DRDY_PIN);


const long baud =  115200;

const long LOADCELL_OFFSET =-12427.400;
const float LOADCELL_DIVIDER_N = -528.894897;
const float LOADCELL_DIVIDER_kg = LOADCELL_DIVIDER_N * 9.81;
const float LOADCELL_DIVIDER_lb = LOADCELL_DIVIDER_N * 4.448;

TaskHandle_t DisplayTask;
TaskHandle_t appendMeasurementTask;
TaskHandle_t getMeasurementTask;
QueueHandle_t displayQueue;
QueueHandle_t sdQueue;

File dataFile;

unsigned long timestamp = 0;
long maxForce = 0;
long force = -1;
long reading = -1;
long avg_reading = 0;
long prevForce = -100;
long prevMaxForce = -100;

int currentSPS = 0; // current samples per second setting
int dataDelayMs = 1; //for getMeasurement scheduling


// TODO: should be selectable with buttons, maybe a small menu?
bool recording = true;
// Set in init_sd, signals readiness to use the data.txt file on the sd.
// This allows the program to run without an sd card connected, without trying to write to it all the time.
bool sd_ready = false;

int readingID = 0;

unsigned long timeNow = 0;

bool Switch_state = false;

void setup() {
  Serial.begin(baud);
  delay(2000);
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

  #ifndef BOARD_XIAO_S3
  displayInit();
  #endif

  init_sd();
    delay(SD_START_DELAY);

  spiADS1220.begin(ADS1220_SCLK_PIN, ADS1220_MISO_PIN, ADS1220_MOSI_PIN, ADS1220_CS_PIN);
  if (!loadcell.init()) {
    Serial.println("ADS1220 not found!");
    while (1);
  }
  Serial.println("ADS1220 connected!");
  loadcell.setCompareChannels(ADS1220_MUX_1_2);
  loadcell.setGain(ADS1220_GAIN_128);
  loadcell.setVRefSource(ADS1220_VREF_REFP1_REFN1);
  loadcell.setVRefValue_V(3.3);
  loadcell.setConversionMode(ADS1220_CONTINUOUS);
  pinMode(ADS1220_DRDY_PIN, INPUT);
  loadcell.setLowSidePowerSwitch(ADS1220_SWITCH);  
  loadcell.setFIRFilter(ADS1220_50HZ_60HZ);
  setSampleRate(false,6); // 1 kHz

  displayQueue = xQueueCreate(1, sizeof(long));

  if(!displayQueue){
    Serial.println("Error creating the display queue");
  }

  sdQueue = xQueueCreate(128, sizeof(LogEntry)); 

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


    if (sd_ready){
  xTaskCreatePinnedToCore(
    appendMeasurement, "appendMeasurementTask", 
    8192, 
    NULL, 
    10, 
    &appendMeasurementTask, 
    0);
    }
  
    xTaskCreatePinnedToCore(
      getMeasurement, "getMeasurementTask", 
      8192, 
      NULL, 
      10, 
      &getMeasurementTask, 
      1);

  displayClearBuffer();
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

  if(!SD.begin(SD_CS_PIN,SPI, 40000000)) {
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
}

void Display(void * parameter) {
  TickType_t lastWakeTime = xTaskGetTickCount();

  for(;;){
    xQueueReceive(displayQueue, &force, portMAX_DELAY);
    Serial.println(force);
    if (force != prevForce) {
      prevForce = force;
      displayForce(force);
    }
    if(maxForce != prevMaxForce) {
      prevMaxForce = maxForce;
      displayMaxForce(maxForce);
    }
    vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(100)); // display at 10 Hz
  }
}

void getMeasurement(void * parameter){
  TickType_t lastWakeTime = xTaskGetTickCount();

  for (;;) {

    int reps = (currentSPS > 1000) ? 2 : 1;
    for (int i = 0; i < reps; i++) {
    long raw2 = loadcell.getRawData();
    float force2 = (raw2 - LOADCELL_OFFSET) / LOADCELL_DIVIDER_N;
    // i made these local to the function and in float because i was getting weird logs of 0.0 / nan and didn't know why
    timeNow = micros();
    maxForce = max(long(force2), maxForce); 

    LogEntry entry = { readingID++, timeNow, force2};
    Serial.print("Force: ");
    Serial.println(force2);

    // only push data into queue (which has size of 1) when it has been emptied by the display task. keep a running average 
    static float avgForceAccumulator = 0;
    static int avgForceCount = 0;

    if (uxQueueMessagesWaiting(displayQueue) == 0) {
        float forceToSend;

      if (avgForceCount > 0) {
        forceToSend = avgForceAccumulator / avgForceCount;
        avgForceAccumulator = 0;
        avgForceCount = 0;
      } else {
        forceToSend = force2;
      }

      long forceLong = long(forceToSend);
     xQueueSend(displayQueue, &forceLong, 0);
    } else {
      // Display hasn't pulled the last value yet — accumulate running average
      avgForceAccumulator += force2;
      avgForceCount++;
    }

    if (sd_ready && recording){
    if (xQueueSend(sdQueue, &entry, 0) != pdTRUE) {
      // Queue full — remove oldest
      LogEntry dummy;
      
      if (xQueueReceive(sdQueue, &dummy, 0) == pdTRUE) {
        // Try again — should succeed now
        if (xQueueSend(sdQueue, &entry, 0) != pdTRUE) {
          // This would only fail in rare timing situations
          Serial.println("⚠️ Failed to replace old data");
          droppedlogs++;
        }
      }
      droppedlogs++;
    
      float secondsRunning = millis() / 1000.0;
      float avgDropsPerSecond = droppedlogs / secondsRunning;
    
      Serial.print("Dropped sample ");
      Serial.print(droppedlogs);
      Serial.print(" | Average drops/sec: ");
      Serial.println(avgDropsPerSecond, 2);
    }
    }
  }
    vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(dataDelayMs)); 

  }
}

void appendMeasurement(void * parameter) {
  LogEntry batch[SD_BATCH_SIZE];
  int count = 0;
  int flushCounter = 0;  // Count how many batches since last flush

  dataFile = SD.open(CSV_NAME, FILE_APPEND);
  if (!dataFile) {
    Serial.println("Failed to open file in writer task.");
    vTaskDelete(NULL);
    return;
  }

  while (true) {
    if (xQueueReceive(sdQueue, &batch[count], portMAX_DELAY)) {
      count++;
      if (count >= SD_BATCH_SIZE) {  
        // going to need to handle when recording goes FALSE because the data left won't hit SD_BATCH_SIZE
        // also pause this task when it isn't needed
        dataFile.write((uint8_t*)batch, sizeof(batch));
        count = 0;
        flushCounter++;

        if (flushCounter >= 4) {  // flush every 4 batches
          dataFile.flush();
          flushCounter = 0;
        }
      }
    }
  }
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



void setSampleRate(bool turbo, uint8_t rateLevel) {
  const ads1220DataRate rateMap[7] = {
    ADS1220_DR_LVL_0, ADS1220_DR_LVL_1, ADS1220_DR_LVL_2,
    ADS1220_DR_LVL_3, ADS1220_DR_LVL_4, ADS1220_DR_LVL_5,
    ADS1220_DR_LVL_6
  };

  const int spsTable[2][7] = {
    {20, 45, 90, 175, 330, 600, 1000},   // Normal
    {40, 90, 180, 350, 660, 1200, 2000}  // Turbo
  };

  if (rateLevel > 6) rateLevel = 6;

  loadcell.setOperatingMode(turbo ? ADS1220_TURBO_MODE : ADS1220_NORMAL_MODE);
  delay(100);
  loadcell.setDataRate(rateMap[rateLevel]);

  currentSPS = spsTable[turbo ? 1 : 0][rateLevel];
  dataDelayMs = (1000 + currentSPS - 1) / currentSPS;  // round up
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