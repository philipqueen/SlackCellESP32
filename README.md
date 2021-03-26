# SlackCellESP32
Development for ESP32 version of SlackCell (original version by Markus Rampp - https://markusrampp.eu/SlackCell/).

Current features are limited to displaying current and max force on integrated OLED, and writing forces to microSD.

Currently reads at ~65 Hz, with significant slowdowns for display.

Future improvements:
- Add BLE (Bluetooth Low Energy) connectivity to allow app development
- Add switch functionality to turn off display, in order to improve sampling rate
- Implement dual core functionality to decrease display slowdowns

Developed for the Heltec Wifi Kit 32. Using other ESP microcontrollers may require changing the U8G2 setup code.

Dual core version is NOT FUNCTIONING. I appreciate any insight into why.
