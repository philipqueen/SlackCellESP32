/* File pins.h  
This file contains the pin definitions for the different supported boards. 
The BOARD_xxxx define is set by the PLATFORM IO build system, look in platformio.ini under build-flags of your board.
*/

//Wrapper to include this file only once
#ifndef PINS_SEEN
#define PINS_SEEN

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
const uint8_t SWITCH_PIN = 22;
const uint8_t SWITCH_MODE = INPUT;

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
const uint8_t SWITCH_PIN = 2;
const uint8_t SWITCH_MODE = INPUT_PULLUP;
#endif
#endif /* !PINS_SEEN */