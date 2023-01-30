# SlackCellESP32
Development for ESP32 version of SlackCell (original version by Markus Rampp - https://markusrampp.eu/SlackCell/).

***Use SlackCellu8g2Switch.ino*** - this is the most current functioning version.

## Code Notes:
Current features are displaying current and max force on integrated OLED, and writing forces to microSD.

Current sampling rate:
- 83 Hz at resting tension/with display off (HX711 is limited to 80Hz)
- 30 Hz with changing force

Recent improvements:
- Switched to the u8g2 library to allow for a hardware I2C connection (was previously software connection)
- Increased display bus clock speed to 1MHz, leading to significant increase in sampling rate
- Added toggle switch to turn off display, allowing 80Hz recording to SD card
- Change only force or max force, never both at the same time, to decrease display slowdowns

Future improvements:
- Add BLE (Bluetooth Low Energy)/WiFi connectivity to allow app development
- Implement dual core functionality to decrease display slowdowns
- Set force change threshold for the display changes (0.1kN)

Developed for the Heltec Wifi Kit 32 v2. Be sure not to buy the v3 version, as the pinout has changed and the PCB will not function properly. Using other ESP32 microcontrollers may require changing the U8G2 setup code.

## How to make a SlackCell ESP32:

Making your own SlackCell requires about $100 worth of parts, a soldering iron, and very minimal soldering experience. The process is as easy as soldering the pin headers to the boards, soldering the boards and loadcell wires to the pcb, and screwing the eye bolts into the load cell.

Before use, a calibration sketch must be uploaded to the microcontroller and the loadcell has to be calibrated with a known weight. The calibration data has to be changed in the slackcell code, and then it can be uploaded to the board *voila!* you have your own dynamometer!

### Approximate price:

The price of the PCBs are still being worked out as they are developed, but they will likely be quite cheap compared to the other costs. The price of all other components is subject to change, but I've included a rough pricing estimate for project planning.

- Heltec Wifi Kit 32 v2: ~$20
- S Beam Load Cell: ~$40
- HX711 Loadcell Amplifier: ~$7 (3 pack)
- MicroSD Card Reader: ~$7 (5 pack)
- M16 Eye Bolts: ~$17

Total: $91 + PCB

Because two of the parts come in bulk, a group of three friends could each build their own (splitting the amplifiers and card readers) for about $82 each, plus the price of three PCBs.

### Parts list:
Links to parts list are NOT affiliate links. Sourcing of the PCB will be added once PCB development reaches a more stable place.

1. Microcontroller: [Buy Heltec Wifi Kit 32 v2 on Amazon](https://www.amazon.com/MakerFocus-Development-0-96inch-Display-Compatible/dp/B076KJZ5QM/)

Heltec has released a v3 of this board, but has changed the pinout, which will cause the existing code and circuit layout to fail. We will update these designs to V3 eventually, but are waiting to ensure support for the new board is good enough.

2. Load Cell: [Buy Load Cell on Amazon](https://www.amazon.com/Portable-High-Precision-Pressure-Tension-Weighing/dp/B077YHFCX4/)

For slackline purposes, an S Beam load cell rated to 2000 kg is recommended. Always back up the load cell with a stronger component, and do not exceed its rated force. Amazon sourcing of these is sporadic, please file an issue if the link above is incorrect.

3. HX711 Loadcell Amplifier: [Buy HiLetGo HX711 on Amazon](https://www.amazon.com/HiLetgo-Weighing-Dual-Channel-Precision-Pressure/dp/B00XRRNCOO)

This comes in a pack of 3. Buying from HiLetGo ensures the amplifier fits properly into the PCB, other HX711s may not fit in the PCB.

4. MicroSD Card Reader: [Buy HiLetGo MicroSD card reader on Amazon](https://www.amazon.com/HiLetgo-Adater-Interface-Conversion-Arduino/dp/B07BJ2P6X6/)

This comes in a pack of 5. Buying from HiLetGo ensures the card reader fits properly into the PCB, other card readers may not fit in the PCB.

5. M16 Eye Bolts: [Buy M16 Eye Bolts on Amazon](https://www.amazon.com/gp/product/B07G1TND28/)

Any M16 bolts should fit, but a short bolt length is desired so they can be threaded all the way onto the load cell. The included lock washers should be used between the bolt and the load cell. 
