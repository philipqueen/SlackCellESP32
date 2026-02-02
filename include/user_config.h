/*
Config file for variables that will be different for every device

Important: Please only commit structural changes to this file, no edited values (unless other default value is desired)
If git pull fails due to local changes in this file solve with the stash:

```
git stash push -m "user_config" -- include/user_config.h
git pull
git stash pop #Now resolve any potential merge conflicts
```

*/

//Wrapper to include this file only once
#ifndef USER_CONFIG_SEEN
#define USER_CONFIG_SEEN

#include "Arduino.h"

//These values must be calibrated for every loadcell to obtain any useful force reading
const long LOADCELL_OFFSET = 2330;
const float LOADCELL_DIVIDER_N = -232;

#ifdef HAS_BATTERY_READOUT
//These values should be calibrated for every different battery to obtain accurate battery percentage. 
//Battery percentage is not proportional to battery voltage. The following look up table (LUT) maps voltage to percentage, based upon measurements of the battery draining.
//Use analysis/plotter.py to analyze a recording of a discharge from full to empty battery.
//Look up by multiplying the voltage by 100, rounding to int and subtract min_lut_voltage, that will be your array index
const uint16_t min_lut_voltage = 330;
const uint16_t max_lut_voltage = 434;
const uint8_t battery_percent_lut [] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3, 4, 5, 6, 7, 8, 9, 10, 12, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 57, 59, 61, 63, 64, 66, 67, 69, 70, 72, 73, 75, 76, 77, 79, 80, 81, 82, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 97, 98, 99, 100, 100, 100, 100, 100, 100, 100, 100};
#endif //HAS_BATTERY_READOUT

#endif //USER_CONFIG_SEEN