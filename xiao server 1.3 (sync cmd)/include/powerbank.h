#pragma once
#include "Arduino.h"

namespace powerbank
{
      constexpr int pulseLenght = 10; // how many ms are needed to be registered by powerbank
      extern unsigned long lastActionTimestamp;

      bool buttonPulse();
      void turnOff();
      bool begin();
      void controller();
      int percentCharged();
      int readRawVoltage();
      int voltageToPercent(int voltage);
}