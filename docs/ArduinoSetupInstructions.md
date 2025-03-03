# SlackCell Setup Instructions - Arduino IDE

1. Download Arduino IDE - https://www.arduino.cc/en/software
2. Install Driver (usb to uart bridge vsp drivers)
	1. Download matching system installation from https://www.silabs.com/developer-tools/usb-to-uart-bridge-vcp-drivers?tab=downloads
	2. Install based on steps for your system
	3. Restart the computer
	4. On linux, with your microcontroller plugged in, check if the ports have changed with `ls /dev/cu.*` and you should see `/dev/cu.SLAB_USBtoUART`
	5. On mac, you can run `systemextensionsctl list` and verify you have `com.silabs.cp210x` in the list. Like on linux you can run `ls /dev/cu.*` and should see `/dev/cu.usbserial*`
3. Download `esp32 by Espressif Systems` in the Boards Manager
	1. Go to Tools -> Boards -> Board Manager
	2. Search `espressif`
	3. Under `esp32 by Espressif Systems` click `Download`
4. Set the active board
	1. Go to Tools -> Boards -> esp32
	2. Select `ESP32 Dev Module`
	3. You should see `ESP32 Dev Module` in the upper left of the IDE
	4. If you see the exact model of the board show up (LilyGo T-Display), you can select that directly instead
5. Set the correct port
	1. If the port wasn't actively selected with the board, go to Tools -> Port
	2. Select the port you identified in Step 2
6. Download Dependencies
	1. Go to Tools -> Manage Libraries
	2. Search HX711, find `HX711 by Rob Tillaart <rob.tillaart@gmail.com>`, and click install
	3. Search `tft_espi`, find `TFT_eSPI by Bodmer`, and click Install
		1. Now, go to `Documents/Arduino/libraries/TFT_eSPI` and open `User_setup_Select.h`
		2. Comment line 27, so it looks like `//#include <User_Setup.h>`
		3. Uncomment line 58, so it looks like `#include <User_Setups/Setup25_TTGO_T_Display.h>`
7. Change upload speed
	1. Go to Tools -> Upload Speed: 921600 and change to 115200
	2. This may need to be set for each sketch
8. Flash SlackCell file
9. If using a microSD card, it must be formatted to FAT32