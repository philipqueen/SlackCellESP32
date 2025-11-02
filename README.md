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

Developed to work with different boards. Supported right now are: Heltec Wifi Kit 32 v2/v3 or TTGO T-Display microcontroller. Using other ESP32 microcontrollers may require changing the wiring and adding a new display driver.

## How to use the code
- install VSCode
- install Platform IO plugin in VSCode
- open this repo in VSCode
- open the Platform IO plugin tab
- find the "Project Tasks"
    hit "Upload and Monitor" in the submenu of the board you are using.

## How to make a SlackCell ESP32:

Making your own SlackCell requires about $100 worth of parts, a soldering iron, and very minimal soldering experience. The process is as easy as soldering the pin headers to the boards, soldering the boards and loadcell wires to the pcb, and screwing the eye bolts into the load cell.

Before use, a calibration sketch must be uploaded to the microcontroller and the loadcell has to be calibrated with a known weight. The calibration data has to be changed in the slackcell code, and then it can be uploaded to the board - *voila!* you have your own dynamometer!

### Approximate price:

The price of the PCBs are still being worked out as they are developed, but they will likely be quite cheap compared to the other costs. The price of all other components is subject to change, but I've included a rough pricing estimate for project planning.

- Heltec Wifi Kit 32 v2/v3 or HiLetgo ESP32 LCD WiFi Kit: ~$20
- S Beam Load Cell: ~$40
- HX711 Loadcell Amplifier: ~$7 (3 pack)
- MicroSD Card Reader: ~$7 (5 pack)
- M16 Eye Bolts: ~$17

Total: $91 + PCB

Because two of the parts come in bulk, a group of three friends could each build their own (splitting the amplifiers and card readers) for about $82 each, plus the price of three PCBs.

### Parts list:
Links to parts list are NOT affiliate links. Sourcing of the PCB will be added once PCB development reaches a more stable place.

**Note**
The here mentionend PCB is outdated and only works for the Heltec V2 (not the V3)
But the connections can be made nicely on one of [these boards](https://www.ebay.de/itm/185050188169?_skw=lochrasterplatine&itmmeta=01K00B5WX5TKSS6RXQW55XG4WB&hash=item2b15d9c989:g:IGIAAOSwbYZiamcn). Its possible to squeeze it on a 50x70mm board

1. Microcontroller: 
    - USA: [Buy HiLetgo ESP32 LCD WiFi Kit on Amazon](https://www.amazon.com/dp/B07X1W16QS/)
      This is a specific version of the TTGO T-Display. Other versions of the same board should work, but haven't been tested.
    - DE/EU: [Buy Heltec V3 on Ebay](https://www.ebay.de/itm/174942289832)
      Its another microcontroller, than above. The software supports both, and this one is better for battery powered application

2. Load Cell:
    - USA: [Buy PSD-S1 Load Cell on Amazon](https://www.amazon.com/Portable-High-Precision-Pressure-Tension-Weighing/dp/B077YHFCX4/)
    - DE/EU: [Buy PSD-S1 Loadcell on Amazon](https://www.amazon.de/Baxnore-PSD-wasserfestes-Ma%C3%9Fstabsensor-elektronische/dp/B0F2FWSL24&tag=amzfinder-20?th=1) not verified, original seller not available anymore, but seems to be the same part

For slackline purposes, an S Beam load cell rated to 2000 kg is recommended. Always back up the load cell with a stronger component, and do not exceed its rated force. Amazon sourcing of these is sporadic, please file an issue if the link above is incorrect.

3. HX711 Loadcell Amplifier:
  - USA: [Buy Comimark HX711 on Amazon](https://www.amazon.com/dp/B07X2JZRKB)
        This comes in a pack of 4. Buying this version ensures the amplifier fits properly into the PCB, other HX711s may not fit in the PCB.
  - DE/EU: [Buy Hx711 on Ebay](https://www.ebay.de/itm/174942018664)

5. MicroSD Card Reader: 
    - USA: [Buy HiLetGo MicroSD card reader on Amazon](https://www.amazon.com/HiLetgo-Adater-Interface-Conversion-Arduino/dp/B07BJ2P6X6/)
        This comes in a pack of 5. Buying from HiLetGo ensures the card reader fits properly into the PCB, other card readers may not fit in the PCB.
    - DE/EU: [Buy MicroSD card reader on Ebay](https://www.ebay.de/itm/174940391637)

6. M16 Eye Bolts:
    - USA: [Buy M16 Eye Bolts on Amazon](https://www.amazon.com/gp/product/B07G1TND28/)
    - EU/DE: [Buy M16 Eye Bolts on Ebay](https://www.ebay.de/itm/292368070350)

Any M16 bolts should fit, but a short bolt length is desired so they can be threaded all the way onto the load cell. The included lock washers should be used between the bolt and the load cell. 
