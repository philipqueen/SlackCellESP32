#include "HX711.h"
#include <TFT_eSPI.h>
#include <SPI.h>

// libraries for SD card
#include "FS.h"
#include "SD.h"

// libraries for captive web server
#include <AsyncTCP.h> 
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <esp_wifi.h>

#define SD_CS 2
#define SD_MOSI 26
#define SD_MISO 27
#define SD_SCK 25

#define CSV_NAME "/slackcell.txt"

#define N_TO_KG 9.81
#define N_TO_LB 4.448

#define SD_MESSAGE_LENGTH 60
#define TARE_AVERAGE_TIME 30
#define MAX_TARE_VALUE 30
#define SD_START_DELAY 2000

#define X_CURSOR_START 10
#define LIVE_Y_CURSOR_START 20
#define PEAK_Y_CURSOR_START 60
#define FILL_RECT_WIDTH 180
#define FILL_RECT_HEIGHT 30

#define MAX_CLIENTS 10
#define WIFI_CHANNEL 6
#define DNS_INTERVAL 30

const long baud = 115200;
TaskHandle_t DisplayTask;
QueueHandle_t queue;
TaskHandle_t WebServerTask;

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
long reading = -1;
long prevForce = -100;
int readingID = 0;
unsigned long timeNow = 0;
String sdMessage;

const char *ssid = "captive";
const char *password = NULL;
const IPAddress localIP(4, 3, 2, 1);
const IPAddress gatewayIP(4, 3, 2, 1);
const IPAddress subnetMask(255, 255, 255, 0);
const String localIPURL = "http://4.3.2.1";

const char index_html[] PROGMEM = R"=====(
  <!DOCTYPE html> <html>
    <head>
      <title>ESP32 Captive Portal</title>
      <style>
        body {background-color:#06cc13;}
        h1 {color: white;}
        h2 {color: white;}
      </style>
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
    </head>
    <body>
      <h1>Hello World!</h1>
      <h2>This is a captive portal example. All requests will be redirected here </h2>
    </body>
  </html>
)=====";

DNSServer dnsServer;
AsyncWebServer server(80);


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

  tft.setCursor(X_CURSOR_START, LIVE_Y_CURSOR_START);
  tft.printf("SLACK");
  tft.setCursor(X_CURSOR_START, PEAK_Y_CURSOR_START);
  tft.printf("CELL");

  loadcell.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  loadcell.set_offset(LOADCELL_OFFSET);
  loadcell.set_scale(LOADCELL_DIVIDER_N);
  force = loadcell.get_units(TARE_AVERAGE_TIME);
  if (force < MAX_TARE_VALUE) {
    loadcell.tare();
    force = 0;
  }

  queue = xQueueCreate(5, sizeof(long));

  if(queue == NULL){
    Serial.println("Error creating the queue");
  }

  xTaskCreatePinnedToCore(
    Display, /* Function to implement the task */
    "DisplayTask", /* Name of the task */
    10000,  /* Stack size in words */
    NULL,  /* Task input parameter */
    0,  /* Priority of the task */
    &DisplayTask,  /* Task handle. */
    0); /* Core where the task should run */

  sdMessage.reserve(SD_MESSAGE_LENGTH);

  spiVspi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);  // prevent SD from interfering with screen HSPI connection

  delay(SD_START_DELAY);

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
  appendFile(SD, CSV_NAME, "Reading ID, Time (ms), Force (N) \n");
  file.close();

  startSoftAccessPoint(ssid, password, localIP, gatewayIP);
	setUpDNSServer(dnsServer, localIP);
	setUpWebserver(server, localIP);
	server.begin();

  xTaskCreatePinnedToCore(
    WebServer, /* Function to implement the task */
    "WebServerTask", /* Name of the task */
    10000,  /* Stack size in words */
    NULL,  /* Task input parameter */
    0,  /* Priority of the task */
    &WebServerTask,  /* Task handle. */
    0); /* Core where the task should run */
}

void loop() {
  if (loadcell.is_ready()) {
    reading = loadcell.get_units(1);
    timeNow = millis(); //milliseconds since startup
    xQueueSend(queue, &reading, portMAX_DELAY);
    Serial.printf("Reading: %ld N\n", abs(reading));
    writeSD(readingID, timeNow, reading);
    readingID += 1;
  } 
}

void Display(void * parameter) {
  for(;;){
    Serial.print("checking queue\n");
    xQueueReceive(queue, &force, portMAX_DELAY);
    Serial.println(force);
    if ((force != prevForce)) {
      prevForce = force;
      maxForce = max(force, maxForce);

      tft.fillRect(X_CURSOR_START, LIVE_Y_CURSOR_START, FILL_RECT_WIDTH, FILL_RECT_HEIGHT, TFT_BLACK);
      tft.setCursor(X_CURSOR_START, LIVE_Y_CURSOR_START); // x, y position
      tft.setTextColor(TFT_BLUE, TFT_BLACK);
      tft.printf("live: %ld", force);

      tft.fillRect(X_CURSOR_START, PEAK_Y_CURSOR_START, FILL_RECT_WIDTH, FILL_RECT_HEIGHT, TFT_BLACK); // need to make this extend further right
      tft.setCursor(X_CURSOR_START, PEAK_Y_CURSOR_START);
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.printf("peak: %ld", maxForce);
    }
  }
}

void WebServer(void * parameter) {
  for(;;) {
    dnsServer.processNextRequest();
    delay(DNS_INTERVAL);
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

void setUpDNSServer(DNSServer &dnsServer, const IPAddress &localIP) {
	// Set the TTL for DNS response and start the DNS server
	dnsServer.setTTL(3600);
	dnsServer.start(53, "*", localIP);
}

void startSoftAccessPoint(const char *ssid, const char *password, const IPAddress &localIP, const IPAddress &gatewayIP) {
	// Set the WiFi mode to access point and station
	WiFi.mode(WIFI_MODE_AP);

	// Define the subnet mask for the WiFi network
	const IPAddress subnetMask(255, 255, 255, 0);

	// Configure the soft access point with a specific IP and subnet mask
	WiFi.softAPConfig(localIP, gatewayIP, subnetMask);

	// Start the soft access point with the given ssid, password, channel, max number of clients
	WiFi.softAP(ssid, password, WIFI_CHANNEL, 0, MAX_CLIENTS);

	// Disable AMPDU RX on the ESP32 WiFi to fix a bug on Android
	esp_wifi_stop();
	esp_wifi_deinit();
	wifi_init_config_t my_config = WIFI_INIT_CONFIG_DEFAULT();
	my_config.ampdu_rx_enable = false;
	esp_wifi_init(&my_config);
	esp_wifi_start();
	vTaskDelay(100 / portTICK_PERIOD_MS);  // Add a small delay
}

void setUpWebserver(AsyncWebServer &server, const IPAddress &localIP) {
	//======================== Webserver ========================
	// WARNING IOS (and maybe macos) WILL NOT POP UP IF IT CONTAINS THE WORD "Success" https://www.esp8266.com/viewtopic.php?f=34&t=4398
	// SAFARI (IOS) IS STUPID, G-ZIPPED FILES CAN'T END IN .GZ https://github.com/homieiot/homie-esp8266/issues/476 this is fixed by the webserver serve static function.
	// SAFARI (IOS) there is a 128KB limit to the size of the HTML. The HTML can reference external resources/images that bring the total over 128KB
	// SAFARI (IOS) popup browser has some severe limitations (javascript disabled, cookies disabled)

	// Required
	server.on("/connecttest.txt", [](AsyncWebServerRequest *request) { request->redirect("http://logout.net"); });	// windows 11 captive portal workaround
	server.on("/wpad.dat", [](AsyncWebServerRequest *request) { request->send(404); });								// Honestly don't understand what this is but a 404 stops win 10 keep calling this repeatedly and panicking the esp32 :)

	// Background responses: Probably not all are Required, but some are. Others might speed things up?
	// A Tier (commonly used by modern systems)
	server.on("/generate_204", [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });		   // android captive portal redirect
	server.on("/redirect", [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });			   // microsoft redirect
	server.on("/hotspot-detect.html", [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });  // apple call home
	server.on("/canonical.html", [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });	   // firefox captive portal call home
	server.on("/success.txt", [](AsyncWebServerRequest *request) { request->send(200); });					   // firefox captive portal call home
	server.on("/ncsi.txt", [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });			   // windows call home

	// B Tier (uncommon)
	//  server.on("/chrome-variations/seed",[](AsyncWebServerRequest *request){request->send(200);}); //chrome captive portal call home
	//  server.on("/service/update2/json",[](AsyncWebServerRequest *request){request->send(200);}); //firefox?
	//  server.on("/chat",[](AsyncWebServerRequest *request){request->send(404);}); //No stop asking Whatsapp, there is no internet connection
	//  server.on("/startpage",[](AsyncWebServerRequest *request){request->redirect(localIPURL);});

	// return 404 to webpage icon
	server.on("/favicon.ico", [](AsyncWebServerRequest *request) { request->send(404); });	// webpage icon

	// Serve Basic HTML Page
	server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request) {
		AsyncWebServerResponse *response = request->beginResponse(200, "text/html", index_html);
		// response->addHeader("Cache-Control", "public,max-age=31536000");  // save this file to cache for 1 year (unless you refresh)
		request->send(response);
		Serial.println("Served Basic HTML Page");
	});

	// the catch all
	server.onNotFound([](AsyncWebServerRequest *request) {
		request->redirect(localIPURL);
		Serial.print("onnotfound ");
		Serial.print(request->host());	// This gives some insight into whatever was being requested on the serial monitor
		Serial.print(" ");
		Serial.print(request->url());
		Serial.print(" sent redirect to " + localIPURL + "\n");
	});
}