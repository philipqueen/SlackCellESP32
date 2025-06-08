#include <Arduino.h>
#include "pins.h"

#ifdef USE_SLEEP
#include "OneButton.h"
#include "driver/rtc_io.h"
#include "display.h"
#include "power.h"


//in minutes
const int STANDBY_TIME = 5;
//in newtons
const int STANDBY_UNDER_FORCE = 15;

#ifdef USE_SLEEP
const unsigned long MIN_POWER_BUTTON_DURATION_FOR_WAKEUP = 1500;
const unsigned long MIN_POWER_BUTTON_DURATION_FOR_SHUTDOWN = 2000;

#ifdef HAS_BATTERY_READOUT
const float MIN_OPERATING_BAT_VOL = 3.3; //In V, MCU will shut down if battery voltage drops below this value
const float CRITICAL_BAT_VOL = 3.1; //Do nothing if the voltage drops below this, not even display the warning about low battery
const unsigned long BATTERY_READOUT_INTERVAL = 1000;
#endif //HAS_BATTERY_READOUT
#endif //SLEEP

long lastTimeOverStandbyLimit = 0;
bool wakingUp = false; //set by power button callback (wakeUp)

//could be controlled later by the user
//if set from false to true, lastTimeOverStandbyLimit must be set before to millis() to prevent immediate standby
bool automaticStandbyActive = true;

OneButton power_btn(POWER_BUTTON_PIN, true);

void goToSleep();

void wakeUp(){
  wakingUp = true;
}
#endif //USE SLEEP

#ifdef USE_VEXT
void VextON(void);
void VextOFF(void);
#endif


/// This function get's called directly after the mcu got reset or waked out of sleep.
/// If the mcu woke up from sleep through a button press, the button needs to be held down for a while longer, to really start the SlackCell.
/// This prevents accidental power on from short bump of the power button.
/// It will also refuse to boot if the battery volage is too low.
void maybeWakeUp(){
#ifdef USE_SLEEP
  if(esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0){
    //A press of the power button woke up the mcu.
#ifdef HAS_BATTERY_READOUT
    if(readBatLevel() < CRITICAL_BAT_VOL){
      //if the battery is really low, go back to sleep directly, without using any more energy on awaiting a long button press or turning on the display
      goToSleep();
    }
#endif
    //To be sure the user really wants to turn the SlackCell on, a long press of the power button is expected
    power_btn.attachLongPressStart(wakeUp);
    power_btn.setPressMs(MIN_POWER_BUTTON_DURATION_FOR_WAKEUP);

    //Keep checking the button during a timeout a little bit longer than the expected duration of the long press of the power button
    //directly after wakeup, millis() is always zero, so we can use absolute values instead of comparing to a timestamp here
    while(millis() < MIN_POWER_BUTTON_DURATION_FOR_WAKEUP + 500){
      power_btn.tick();
      if(wakingUp){ //"wakingUp" gets set to true as sideeffect of the callback for the power_btn set above
#ifdef HAS_BATTERY_READOUT
        if(readBatLevel() < MIN_OPERATING_BAT_VOL){
          //but if it just crossed the threshold, inform the user about the low battery
          displayInit();
          displayBatteryLow();
          delay(1500);
          goToSleep();
        }
#endif //HAS_BATTERY_READOUT
        return; //continue booting
        lastTimeOverStandbyLimit = millis();
      }
    }
    //if specified time passes without detecting a long press, go back to sleep
    goToSleep();
  }
  //on other wakeup reasons always wake up
#endif
}

void powerInit(){
    maybeWakeUp();

#ifdef USE_VEXT
    //turn on external devices
    VextON();
#endif //USE_VEXT

#ifdef USE_SLEEP
    power_btn.attachLongPressStart(goToSleep);
    power_btn.setPressMs(MIN_POWER_BUTTON_DURATION_FOR_SHUTDOWN);
#endif
}

void powerTick(long reading){
#ifdef USE_SLEEP
    unsigned long timeNow = millis();

    power_btn.tick();

#ifdef HAS_BATTERY_READOUT
    static unsigned long last_battery_readout_time;

    if(millis() - last_battery_readout_time > BATTERY_READOUT_INTERVAL){
      last_battery_readout_time = timeNow;
      if(readBatLevel() < MIN_OPERATING_BAT_VOL){
        displayBatteryLow();
        delay(1500);
        goToSleep();
      }
    }
#endif //HAS_BATTERY_READOUT
    
    if(!automaticStandbyActive){
      return;
    }

    if(reading > STANDBY_UNDER_FORCE){
      lastTimeOverStandbyLimit = timeNow;
    } else if(timeNow - lastTimeOverStandbyLimit > 1000 * 60 * STANDBY_TIME){
      goToSleep();
    }
#endif
}

#ifdef USE_SLEEP
void goToSleep(){
  Serial.println("Getting ready for sleeping ...");
  // Turn off display
  displaySleep();
#ifdef USE_VEXT
  VextOFF();
#else
  // Put hx711 to sleep, only needed if not controlled through external power
  digitalWrite(LOADCELL_SCK_PIN, HIGH);
#endif

  esp_sleep_enable_ext0_wakeup(POWER_BUTTON_PIN, LOW);
  // Configure pullup/downs via RTCIO to tie wakeup pins to inactive level during deepsleep.
  // EXT0 resides in the same power domain (RTC_PERIPH) as the RTC IO pullup/downs.
  // No need to keep that power domain explicitly, unlike EXT1.
  rtc_gpio_pullup_en(POWER_BUTTON_PIN);
  rtc_gpio_pulldown_dis(POWER_BUTTON_PIN);
  Serial.flush();

  // Delaying the final call to sleep, to prevent bouncing of the button from waking up the device directly again.
  // By this time user probably got bored pressing the button and button bouncing should be over.
  delay(1000);
  Serial.printf("Deep sleeping until power button is pressed again");

  esp_deep_sleep_start();
}
#endif //USE_SLEEP

#ifdef HAS_BATTERY_READOUT
//The Heltec V3 has a voltage divider connected to the battery, which needs to be activated with another GPIO pin.
//Schematic for the Heltec V3, relevant is the bottom left section: https://resource.heltec.cn/download/WiFi_Kit_32_V3/HTIT-WB32_V3_Schematic_Diagram.pdf
float readBatLevel() {
  pinMode(VBAT_READ_CONTROL_PIN, OUTPUT);
  digitalWrite(VBAT_READ_CONTROL_PIN, LOW);
  
  analogSetPinAttenuation(VBAT_ADC_PIN, VBAT_ADC_ATTENUATION);
  float voltage = analogReadMilliVolts(VBAT_ADC_PIN) * VBAT_CONVERSION_FACTOR;

  pinMode(VBAT_READ_CONTROL_PIN, INPUT);
  return voltage;
}
#endif //HAS_BATTERY_READOUT


#ifdef USE_VEXT
// Turn external power supply on
void VextON(void)
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, LOW);
}

//Turn external power supply off
void VextOFF(void) //Vext default OFF
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, HIGH);
}
#endif //USE_VEXT