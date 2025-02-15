//only use this display implementation for Heltec boards
#if defined(BOARD_HELTEC_V2) || defined(BOARD_HELTEC_V3)

#include <Arduino.h>

#include "display.h"
#include "pins.h"
#include "U8g2lib.h" //library for OLED

U8G2_SSD1306_128X64_NONAME_F_HW_I2C   u8g2(U8G2_R2, OLED_RESET_PIN, OLED_CLOCK_PIN, OLED_DATA_PIN); //setup display connection

void display_six_digits(long number, uint8_t line){
  // Maximum number of letters the screen can display in one row
  uint8_t maxLength = 6;
  // Long range is +-2,147,483,648
  char bufferF[12] = {};
  ltoa(number, bufferF, 10);
  // align right by padding with spaces
  char bufferLCD[maxLength + 1] = {' ', ' ', ' ', ' ', ' ', ' ', '\0'};
  for (int i = 0; i < strlen(bufferF); i++) {
    bufferLCD[maxLength - strlen(bufferF) + i] = bufferF[i];
  }
  u8g2.drawStr(0, line, bufferLCD);
  u8g2.sendBuffer();
}

void displayForce(long force) {
  display_six_digits(force, 0);
}

void displayMaxForce(long force) {
  display_six_digits(force, 36);
}

void displayInit(){
  u8g2.setBusClock(1000000);
  u8g2.begin();
  u8g2.setPowerSave(0);
  u8g2.setFont(u8g2_font_inb21_mf);
  u8g2.setFontPosTop();


  u8g2.clearBuffer();
  u8g2.drawStr(0, 0, "SLACK");
  u8g2.drawStr(0, 36, "CELL");
  u8g2.sendBuffer();
}

void displayClearBuffer(){
  u8g2.clearBuffer();
}

#endif //Board filter