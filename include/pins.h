/* File pins.h  
This file contains the pin definitions for the different supported boards. 

BOARD_xxxx: This define is set by the PLATFORM IO build system, look in platformio.ini under build-flags of your board.
USE_VEXT: if defined, enables usage of switchable external voltage. Only enable if support on your specific board
    Using this supply for connected devices, like HX711 and SD card will reduce power consumption in deep sleep.
    If the used dev board does not support switching external power, this functionality can be achieved by adding a mosfet circuit and defining the variable "Vext" to the pin of the mosfet controlling the external power.
HAS_BATTERY_READOUT: define if your board/circuit does support reading out the battery voltage
USE_RESET_BUTTON: if defined, enables usage of a button to toggle display updates and reset peak force with a long press
USE_SWITCH: if defined, enables usage of a switch to toggle display updates
USE_INFO_BUTTON: if defined, enables showing battery information on press of a button, depending on HAS_BATTERY_READOUT
USE_VSPI: if defined, enables usage of the VSPI bus for the SD card. This is required for some boards with SPI
    screen interfaces to prevent SPI conflicts with the SD card
CUSTOM_SPI_PINS: if defined, enables usage of custom SPI pins for the screen. This is required for boards that do not 
    use the default SPI pins for the screen connection
READING_AVG_TIMES: number of readings to average for the HX711. 
    For boards where the display cannot keep up with readings, average values to send to the display.
    Set to 1 to disable averaging.
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
#define USE_SWITCH
const uint8_t SWITCH_PIN = 22;
const uint8_t SWITCH_MODE = INPUT;

//The Heltec Wifi Kit V2 has this feature, but it is not wired to use it on the custom pcb
//#define USE_VEXT
#define READING_AVG_TIMES 7

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
#define USE_RESET_BUTTON
const uint8_t RESET_BUTTON_PIN = 4;
#define USE_INFO_BUTTON
const uint8_t INFO_BUTTON_PIN = 3;
#define USE_SLEEP
const gpio_num_t POWER_BUTTON_PIN = GPIO_NUM_2;

#define HAS_BATTERY_READOUT
//Schematic for the Heltec V3, relevant is the bottom left section: https://resource.heltec.cn/download/WiFi_Kit_32_V3/HTIT-WB32_V3_Schematic_Diagram.pdf
const uint8_t VBAT_ADC_PIN  = 1;
const uint8_t VBAT_READ_CONTROL_PIN = 37; // Heltec V3 GPIO to toggle VBatt read connection, this pin enables disabling the voltage divider connecting the battery voltage and therefore conserving power, especially important in sleep mode
const adc_attenuation_t VBAT_ADC_ATTENUATION = ADC_2_5db; //For the Heltec V3 this limits the measuring range to 1.05V which is enough, after the battery voltage passed through the voltage divider, see: https://docs.espressif.com/projects/arduino-esp32/en/latest/api/adc.html#analogsetattenuation
const float VBAT_CONVERSION_FACTOR = 0.001f / (100.0f / (100.0f + 390.0f)); //0.001 converts from mV to V, the other values come from the builtin voltage divider, which has the resistors R1 = 390kOhm and R2 = 100kOhm

#define USE_VEXT
#define READING_AVG_TIMES 7


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
#define USE_RESET_BUTTON
const uint8_t RESET_BUTTON_PIN = 0;

#define USE_VSPI
#define READING_AVG_TIMES 1

#endif //BOARD_TTGO_DISPLAY

//Checks
#if defined(USE_INFO_BUTTON) && !defined(HAS_BATTERY_READOUT)
#error "Info feature only works when battery readout is supported!"
#endif

#endif //PINS_SEEN