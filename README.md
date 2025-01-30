# SlackCellESP32
Development for ESP32 version of SlackCell.

Based on the original SlackCell by [Markus Rampp](https://markusrampp.eu/SlackCell/). Markus's version used a much more expensive microcontroller, and was optimized for driving LEDs in response to tension changes. This version is optimized to be:
- Easy and cheap to build, by requiring very few components
- Great for standard slackline needs (measuring your line tension for parklines or highlines)
- Suitable for slackline science (measurement speed as fast as possible with readily available parts)

First calibrate your loadcell by loading *Calibration.ino*, and use the values displayed with your known weight to alter the calibration values in *SlackCell.ino*. Then load and run the latter file with your updated values, and you have a dyno!

## Code Notes:
Current features are displaying current and max force on integrated OLED, and writing forces to microSD.

Current sampling rate:
- Need to retest sampling rates for new hardware
- ~~83 Hz at resting tension/with display off (HX711 is limited to 80Hz)~~
- ~~30 Hz displaying changing force~~

Recent improvements:
- Switched to new TTGO T-Display hardware (from HiLetGO) 
- Switched from U8g2 to TFT_eSPI for new colored display - still need to test sampling rates
- Increased display bus clock speed to 1MHz, leading to significant increase in sampling rate
- ~~Added toggle switch to turn off display, allowing 80Hz recording to SD card~~ Need to reimplement with integrated buttons on TTGO T-Display
- Change only force or max force, never both at the same time, to decrease display slowdowns

Future improvements:
- Add BLE (Bluetooth Low Energy)/WiFi connectivity to allow app development
- Run display logic on separate core from force reading and SD writing (ESP32 is dual core)
- Set force change threshold for the display changes (0.1kN)
- Get a production quality PCB made for the project

Developed for the TTGO T-Display microcontroller, sold under a few brands including HiLetGo. 
Using other ESP32 microcontrollers may require changing the wiring and the display code.

## How to make a SlackCell ESP32:

Making your own SlackCell requires about $100 worth of parts, a soldering iron, and very minimal soldering experience. The process is as easy as soldering the pin headers to the boards, soldering the boards and loadcell wires to the pcb, and screwing the eye bolts into the load cell.

Before use, a calibration sketch must be uploaded to the microcontroller and the loadcell has to be calibrated with a known weight. The calibration data has to be changed in the slackcell code, and then it can be uploaded to the board - *voila!* you have your own dynamometer!

### Approximate price:

The price of the PCBs are still being worked out as they are developed, but they will likely be quite cheap compared to the other costs. The price of all other components is subject to change, but I've included a rough pricing estimate for project planning.

- HiLetgo ESP32 LCD WiFi Kit: ~$20
- S Beam Load Cell: ~$40
- HX711 Loadcell Amplifier: ~$7 (3 pack)
- MicroSD Card Reader: ~$7 (5 pack)
- M16 Eye Bolts: ~$17

Total: $91 + PCB

Because two of the parts come in bulk, a group of three friends could each build their own (splitting the amplifiers and card readers) for about $82 each, plus the price of three PCBs.

### Parts list:
Links to parts list are NOT affiliate links. Sourcing of the PCB will be added once PCB development reaches a more stable place.

1. Microcontroller: [Buy HiLetgo ESP32 LCD WiFi Kit on Amazon](https://www.amazon.com/dp/B07X1W16QS/)

This is a specific version of the TTGO T-Display. Other versions of the same board should work, but haven't been tested.

2. Load Cell: [Buy Load Cell on Amazon](https://www.amazon.com/Portable-High-Precision-Pressure-Tension-Weighing/dp/B077YHFCX4/)

For slackline purposes, an S Beam load cell rated to 2000 kg is recommended. Always back up the load cell with a stronger component, and do not exceed its rated force. Amazon sourcing of these is sporadic, please file an issue if the link above is incorrect.

3. HX711 Loadcell Amplifier: [Buy WWZMDiB HX711 on Amazon](https://www.amazon.com/AiTrip-Weighing-Conversion-Sensors-Microcontroller/dp/B07SGPX7ZH)

This comes in a pack of 4. Buying this version ensures the amplifier fits properly into the PCB, other HX711s may not fit in the PCB.

4. MicroSD Card Reader: [Buy HiLetGo MicroSD card reader on Amazon](https://www.amazon.com/HiLetgo-Adater-Interface-Conversion-Arduino/dp/B07BJ2P6X6/)

This comes in a pack of 5. Buying from HiLetGo ensures the card reader fits properly into the PCB, other card readers may not fit in the PCB.

5. M16 Eye Bolts: [Buy M16 Eye Bolts on Amazon](https://www.amazon.com/gp/product/B07G1TND28/)

Any M16 bolts should fit, but a short bolt length is desired so they can be threaded all the way onto the load cell. The included lock washers should be used between the bolt and the load cell. 
