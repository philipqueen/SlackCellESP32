//only use this display implementation for TFT compatible boards
#if defined(BOARD_TTGO_DISPLAY)

#include <Arduino.h>

#include "display.h"
#include "pins.h"
#include <TFT_eSPI.h> 

#define X_CURSOR_START 10
#define X_CURSOR_NUMBER_START 120
#define LIVE_Y_CURSOR_START 20
#define PEAK_Y_CURSOR_START 60
#define FILL_RECT_WIDTH 60
#define FILL_RECT_HEIGHT 30

TFT_eSPI tft = TFT_eSPI(); // unfortunately the pins must be set in the library file manually, and cant be set programmatically

void displayForce(long force) {
  tft.fillRect(X_CURSOR_NUMBER_START, LIVE_Y_CURSOR_START, FILL_RECT_WIDTH, FILL_RECT_HEIGHT, TFT_BLACK);
  tft.setCursor(X_CURSOR_NUMBER_START, LIVE_Y_CURSOR_START);
  tft.setTextColor(TFT_BLUE, TFT_BLACK);
  tft.printf("%ld", force);
}

void displayMaxForce(long force) {
  tft.fillRect(X_CURSOR_NUMBER_START, PEAK_Y_CURSOR_START, FILL_RECT_WIDTH, FILL_RECT_HEIGHT, TFT_BLACK); // need to make this extend further right
  tft.setCursor(X_CURSOR_NUMBER_START, PEAK_Y_CURSOR_START);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.printf("%ld", force);
}

void displayInit(){
  tft.init();
  tft.setRotation(1); // horizontal layout
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(3);

  tft.setCursor(X_CURSOR_START, LIVE_Y_CURSOR_START);
  tft.printf("SLACK");
  tft.setCursor(X_CURSOR_START, PEAK_Y_CURSOR_START);
  tft.printf("CELL");

}

void displayClearBuffer(){
  tft.fillRect(X_CURSOR_START, LIVE_Y_CURSOR_START, FILL_RECT_WIDTH, FILL_RECT_HEIGHT, TFT_BLACK);
  tft.setCursor(X_CURSOR_START, LIVE_Y_CURSOR_START);
  tft.setTextColor(TFT_BLUE, TFT_BLACK);
  tft.print("live:");

  tft.fillRect(X_CURSOR_START, PEAK_Y_CURSOR_START, FILL_RECT_WIDTH, FILL_RECT_HEIGHT, TFT_BLACK);
  tft.setCursor(X_CURSOR_START, PEAK_Y_CURSOR_START);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.print("peak:");
}

#endif //Board filter