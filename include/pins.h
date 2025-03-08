/* File pins.h  
This file contains the pin definitions for the different supported boards. 

BOARD_xxxx: This define is set by the PLATFORM IO build system, look in platformio.ini under build-flags of your board.
USE_VEXT: if defined, enables usage of switchable external voltage. Only enable if support on your specific board
    Using this supply for connected devices, like HX711 and SD card will reduce power consumption in deep sleep
*/

//Wrapper to include this file only once
#ifndef PINS_SEEN
#define PINS_SEEN
#endif

#if defined(BOARD_HELTEC_V2)
// internal OLED wiring
const uint8_t OLED_RESET_PIN = 16;
const uint8_t OLED_CLOCK_PIN = 15;
const uint8_t OLED_DATA_PIN = 4;
#define OLED_ROTATION U8G2_R0
// SD Card circuit wiring
// With the Heltec V2 the board default SPI pins are used SCK = 18, MISO = 19, and MOSI = 23
const uint8_t SD_CS_PIN = 2;
// HX711 circuit wiring
const uint8_t LOADCELL_SCK_PIN = 26;
const uint8_t LOADCELL_DOUT_PIN = 25;
//Controls
#define USE_SWITCH
const uint8_t SWITCH_PIN = 22;
const uint8_t SWITCH_MODE = INPUT;

//The Heltec Wifi Kit V2 has this feature, but it is not wired to use it on the custom pcb
//#define USE_VEXT

#elif defined(BOARD_HELTEC_V3)
// internal OLED wiring
const uint8_t OLED_CLOCK_PIN = 18;
const uint8_t OLED_RESET_PIN = 21;
const uint8_t OLED_DATA_PIN = 17;
#define OLED_ROTATION U8G2_R2
// SD Card circuit wiring
// The Heltec V3 allows usage of any GPIO pins as SPI, these are selected to be short to connect on a prototype grid pcb
#define CUSTOM_SPI_PINS
const uint8_t SD_MISO_PIN = 26;
const uint8_t SD_MOSI_PIN = 48;
const uint8_t SD_SCK_PIN =  47;
const uint8_t SD_CS_PIN = 33;
// HX711 circuit wiring
const uint8_t LOADCELL_SCK_PIN = 40;
const uint8_t LOADCELL_DOUT_PIN = 41;
//Controls
#define USE_BUTTON
const uint8_t BUTTON_PIN = 2;

#define USE_VEXT


#elif defined(BOARD_TTGO_DISPLAY)

#define SCREEN_ROTATION 1
// SD Card circuit wiring
#define CUSTOM_SPI_PINS
const uint8_t SD_MISO_PIN = 27;
const uint8_t SD_MOSI_PIN = 26;
const uint8_t SD_SCK_PIN =  25;
const uint8_t SD_CS_PIN = 2;
// HX711 circuit wiring
const uint8_t LOADCELL_SCK_PIN = 33;
const uint8_t LOADCELL_DOUT_PIN = 32;
#define USE_BUTTON
const uint8_t BUTTON_PIN = 0;

#define USE_VSPI

#endif
