#include <Arduino.h>
#include "pins.h"

#ifdef USE_SLEEP
#include "OneButton.h"
#include "driver/rtc_io.h"
#include "display.h"


//in minutes
const int STANDBY_TIME = 5;
//in newtons
const int STANDBY_UNDER_FORCE = 15;

long lastTimeOverStandbyLimit = 0;
bool wakingUp = false;

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
void maybeWakeUp(){
#ifdef USE_SLEEP
  if(esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0){
    power_btn.attachLongPressStart(wakeUp);
    power_btn.setPressMs(3500);

    long boot_time = millis();
    while(millis() - boot_time < 4000){
      power_btn.tick();
      if(wakingUp){
        return;
        lastTimeOverStandbyLimit = millis();
      }
    }
    //if 4 seconds pass without detecting a long press, go back to sleep
    goToSleep();
  }
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
    power_btn.setPressMs(3000);
#endif
}

void powerTick(long reading){
#ifdef USE_SLEEP
    unsigned long timeNow = millis();

    power_btn.tick();
    
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

  Serial.printf("Deep sleeping until power button is pressed again");
  esp_sleep_enable_ext0_wakeup(POWER_BUTTON_PIN, LOW);
  // Configure pullup/downs via RTCIO to tie wakeup pins to inactive level during deepsleep.
  // EXT0 resides in the same power domain (RTC_PERIPH) as the RTC IO pullup/downs.
  // No need to keep that power domain explicitly, unlike EXT1.
  rtc_gpio_pullup_en(POWER_BUTTON_PIN);
  rtc_gpio_pulldown_dis(POWER_BUTTON_PIN);
  Serial.flush();

  // Delaying the final call to sleep, to prevent bouncing of the button from waking up the device directly again.
  // By this time user probably got bored pressing the button and button bouncing should be over.
  delay(2000);

  esp_deep_sleep_start();
}
#endif //USE_SLEEP

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