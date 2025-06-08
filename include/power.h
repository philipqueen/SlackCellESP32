//Wrapper to include this file only once
#ifndef POWER_SEEN
#define POWER_SEEN

#include "pins.h"

/// Does all the power handling needed on startup
/// Depending on the configuration, this can include:
///  - Handling wake up of deep sleep
///  - Checking for sufficient battery voltage
///  - Setting up button to let the user put the unit to sleep
///  - Controlling external power to connected devices
void powerInit();

/// Should be called in the main loop, to query the button and execute standby if needed.
/// @param reading The last read force, this is needed for the automatic standby.
void powerTick(long reading);

#ifdef HAS_BATTERY_READOUT
float readBatLevel();
#endif //HAS_BATTERY_READOUT

#endif //POWER_SEEN