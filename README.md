# SlackCellESP32
Development for ESP32 version of SlackCell (original version by Markus Rampp - https://markusrampp.eu/SlackCell/).

***Use SlackCellu8g2Switch.ino*** - this is the most current functioning version.

Current features are limited to displaying current and max force on integrated OLED, and writing forces to microSD.

Current sampling rate:
- 83 Hz at resting tension/with display off (HX711 is limited to 80Hz)
- 30 Hz with changing force

Recent improvements:
- Switched to the u8g2 library to allow for a hardware I2C connection (was previously software connection)
- Increased display bus clock speed to 1MHz, leading to significant increase in sampling rate
- Added toggle switch to turn off display, allowing 80Hz recording to SD card
- Change only force or max force, never both at the same time, to decrease display slowdowns

Future improvements:
- Add BLE (Bluetooth Low Energy) connectivity to allow app development
- Implement dual core functionality to decrease display slowdowns
- Set force change threshold for the display changes (0.1kN)

Developed for the Heltec Wifi Kit 32. Using other ESP32 microcontrollers may require changing the U8G2 setup code.

Dual core version is NOT FUNCTIONING. I appreciate any insight into why.
