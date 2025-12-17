#pragma once
#include "Arduino.h"
#include "driver/touch_pad.h"
#include "Timer.h"

class CapacitiveRead
{
public:
  static constexpr unsigned long debounceMs = 1500;
  const int pin, deltaTreshold;
  CapacitiveRead(const int pin, const int deltaTreshold = 100) : pin(pin), deltaTreshold(deltaTreshold)
  {}

  bool read();
};