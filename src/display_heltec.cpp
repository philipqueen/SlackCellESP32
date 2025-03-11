//only use this display implementation for Heltec boards
#if defined(BOARD_HELTEC_V2) || defined(BOARD_HELTEC_V3)

#include <Arduino.h>

#include "display.h"
#include "pins.h"
#include "U8g2lib.h" //library for OLED

U8G2_SSD1306_128X64_NONAME_F_HW_I2C   u8g2(U8G2_R2, OLED_RESET_PIN, OLED_CLOCK_PIN, OLED_DATA_PIN); //setup display connection

//number: number to display
//line: y position on screen to start drawing on
//maxLength: Maximum number of letters the screen can display in one row
void displayNdigits(long number,  uint8_t line, uint8_t maxLength){
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
  displayNdigits(force, 0, 6);
}

void displayMaxForce(long force) {
  displayNdigits(force, 36, 6);
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

void displaySDWorking(bool working){
  // pass
}

#endif //Board filter