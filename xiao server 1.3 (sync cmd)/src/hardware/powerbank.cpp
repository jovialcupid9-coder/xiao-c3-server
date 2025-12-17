#include "prototypes.h"

namespace powerbank
{
      unsigned long lastActionTimestamp = 0;

      int adc_pin = 0;
      int button_pin = 0;
      int autoturnoff_minutes = 0;

      bool begin()
      {
            adc_pin = storage::get("powerbank-adc-pin", 0);
            button_pin = storage::get("powerbank-button-pin", 0);
            autoturnoff_minutes = storage::get("autoTurnOff-minutes", 5);

            return (adc_pin && button_pin && autoturnoff_minutes);
      }

      bool buttonPulse() /* imitate pushing built in powerbank's button to prevent shutdown */
      {
            if (!button_pin)
                  return false;

            digitalWrite(button_pin, HIGH);
            delay(pulseLenght);
            digitalWrite(button_pin, LOW);

            return true;
      }

      void turnOff()
      {
            printer("turn off in 3 sec");
            delay(3000);

            for (int i = 0; i < 3; i++)
                  buttonPulse();

            printer("turnOff err, device still alive! Fallback into deep sleep");
            delay(100); // for uart
            ESP.deepSleep(0);
      }

      void controller() /* keep powerbank alive and turn off when idle */
      {
            if (!button_pin)
                  return;

            static Timer timer(10'000);
            if (timer.timePassed())
            {
                  if (millis() - lastActionTimestamp > 60'000UL * autoturnoff_minutes) // some logic here is fucked up propobly
                        turnOff();
                  else
                        buttonPulse();
            }
      }

      int percentCharged()
      {
            return voltageToPercent(readRawVoltage());
      }

      int readRawVoltage()
      {
            return adc_pin ? analogRead(adc_pin) : 0;
      }

      int voltageToPercent(int voltage)
      {
            return map(voltage, 0, 1023, 0, 100);
            /* implement look up table to increase accuracy */
      }
}