# SlackCellESP32
Development for ESP32 version of SlackCell (original version by Markus Rampp - https://markusrampp.eu/SlackCell/).

***Use SlackCellu8g2.ino*** - this is the most current functioning version.

Current features are limited to displaying current and max force on integrated OLED, and writing forces to microSD.

Current sampling rate:
- 83 Hz at resting tension (HX711 is limited to 80Hz)
- 30 Hz with changing force
- 18 Hz with changing max force

Recent improvements:
- Switched to the u8g2 library to allow for a hardware I2C connection (was previously software connection)
- Increased display bus clock speed to 1MHz, leading to significant increase in sampling rate

Future improvements:
- Add BLE (Bluetooth Low Energy) connectivity to allow app development
- Add switch functionality to turn off display, in order to improve sampling rate
- Implement dual core functionality to decrease display slowdowns
- Set force change threshold for the display changes (0.1kN)

Developed for the Heltec Wifi Kit 32. Using other ESP microcontrollers may require changing the U8G2 setup code.

Dual core version is NOT FUNCTIONING. I appreciate any insight into why.
