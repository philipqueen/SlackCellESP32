# SlackCell Wiring Guide for HiLetGo TTGO T-Display

| Loadcell | HX711      |
| -------- | ---------- |
| Red      | E+         |
| Black    | E-         |
| White    | A+         |
| Green    | A-         |
| Yellow   | Unattached |

| HX711 | hiLetGo | Description                                  |
| ----- | ------- | -------------------------------------------- |
| GND   | GND     |                                              |
| DT    | 32      | In the past used DAC pins, but not necessary |
| SCK   | 33      | In the past used DAC pins, but not necessary |
| VCC   | 5V      |                                              |

| OLED | HiLetGo | Description             |
| ---- | ------- | ----------------------- |
| MOSI | 19      | VSPI MISO               |
| SCLK | 18      | VSPI CLK                |
| CS   | 5       | VSPI CS                 |
| DC   | 16      | Can't find what this is |
| RST  | 23      | VSPI MOSI               |
| BL   | 4       | Can't find what this is |
^these are hard wired to the existing screen for SPI connection

| MicroSD | HiLetGo | Description                                                                                    |
| ------- | ------- | ---------------------------------------------------------------------------------------------- |
| CS      | 2       |                                                                                                |
| SCK     | 25      | Adapted from https://github.com/Xinyuan-LilyGO/TTGO-T-Display/issues/14#issuecomment-584471342 |
| MOSI    | 26      |                                                                                                |
| MISO    | 27      |                                                                                                |
| VCC     | 5V      | Some need 3.3V, test specific models                                                           |
| GND     | GND     |                                                                                                |
