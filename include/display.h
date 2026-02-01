/* File display.h 
This header defines the abstract interface for the display functions we need. The display specific implementation is under src/display_xxxx.cpp
*/

//Wrapper to include this file only once
#ifndef DISPLAY_SEEN
#define DISPLAY_SEEN

void displayForce(long force);
void displayMaxForce(long force);
void displayInit();
void displayClearBuffer();
void displaySleep();
void displayWakeup();
void displayInfo(float batteryVoltage, uint8_t batteryPercent);
void displayBatteryLow();
void displayBatteryIcon(int percent);


#endif //DISPLAY_SEEN

