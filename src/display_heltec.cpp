//only use this display implementation for Heltec boards
#if defined(BOARD_HELTEC_V2) || defined(BOARD_HELTEC_V3)

#include <Arduino.h>

#include "display.h"
#include "pins.h"
#include "U8g2lib.h" //library for OLED

bool initialized = false;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C   u8g2(U8G2_R2, OLED_RESET_PIN, OLED_CLOCK_PIN, OLED_DATA_PIN); //setup display connection

//number: number to display
//line: y position on screen to start drawing on
//x: x position on screen to start drawing on
//maxLength: Maximum number of letters the screen can display in one row
void displayNdigits(long number,  uint8_t line, uint8_t x, uint8_t maxLength){
  // Long range is +-2,147,483,648
  char bufferF[12] = {};
  ltoa(number, bufferF, 10);
  // align right by padding with spaces
  char bufferLCD[maxLength + 1] = {' ', ' ', ' ', ' ', ' ', ' ', '\0'};
  for (int i = 0; i < strlen(bufferF); i++) {
    bufferLCD[maxLength - strlen(bufferF) + i] = bufferF[i];
  }
  u8g2.drawStr(x, line, bufferLCD);
  u8g2.sendBuffer();
}

void displayForce(long force) {
  displayNdigits(force, 0, 21, 5);
}

void displayMaxForce(long force) {
  displayNdigits(force, 36, 21, 5);
}

void displaySetDefaultFont(){
  u8g2.setFont(u8g2_font_inb21_mf);
  u8g2.setFontPosTop();
}

void displayInit(){
  u8g2.setBusClock(1000000);
  u8g2.begin();
  u8g2.setPowerSave(0);
  displaySetDefaultFont();


  u8g2.clearBuffer();
  u8g2.drawStr(0, 0, "SLACK");
  u8g2.drawStr(0, 36, "CELL");
  u8g2.sendBuffer();

  initialized = true;
}


void displayBatteryLow(){
  u8g2.clearBuffer();
  //switching to smaller font, to fit the entire message
  u8g2.setFont(u8g2_font_lubR19_tf);
  u8g2.setFontPosTop();
  u8g2.drawStr(0, 0, "Battery");
  u8g2.drawStr(0, 36, "too low!");
  u8g2.sendBuffer();
  //not resetting font to default, because MCU will sleep, and therefore reset before the next display action anyways
}

//Draws a battery icon in the bottom left corner
//percent: battery percentage, expected input range 0 to 100, everything else will be clamped to those limits
void displayBatteryIcon(int percent){
  static const char BATTERY_SYMBOL_CHARGING = '=';
  static const char BATTERY_SYMBOL_EMPTY = '5';
  static const char BATTERY_SYMBOL_FULL = '<';
  
  percent = constrain(percent, 0, 100);
  
  //Changing font to display battery icons instead of letters: https://github.com/olikraus/u8g2/wiki/fntgrpu8g#battery24
  u8g2.setFont(u8g2_font_battery24_tr);
  u8g2.setFontPosBottom();
  //mapping percent from 0 to 100 to the battery icons in the specific battery font.
  //It only works, because the battery symbols are consecutive in the font
  char display_str[2] = {(char) roundf(percent/100.0f*(BATTERY_SYMBOL_FULL - BATTERY_SYMBOL_EMPTY) + BATTERY_SYMBOL_EMPTY), '\0'};
  u8g2.drawStr(0, 64, display_str);
  u8g2.sendBuffer();

  displaySetDefaultFont();
}

void displayInfo(float batteryVoltage, uint8_t batteryPercent){
  char buffer[15];
  //smaller font to fit more characters
  u8g2.setFont(u8g2_font_lubR18_tf);
  //%% prints the literal character %
  snprintf(buffer, 15, "%.2fV %u%%", batteryVoltage, batteryPercent);
  displayClearBuffer();
  u8g2.drawStr(0, 0, buffer);
  u8g2.sendBuffer();
  displaySetDefaultFont();
}

void displayClearBuffer(){
  u8g2.clearBuffer();
}

void displaySleep(){
  // This function also get's called when waking up from deep sleep,
  // where the display will not be initialized yet
  // This condition prevents error's in this case
  if(initialized){
    u8g2.setPowerSave(1);
  }
}

void displayWakeup(){
  u8g2.setPowerSave(0);
}

#endif //Board filter