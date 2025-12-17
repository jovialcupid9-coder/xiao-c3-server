#include "CapacitiveRead.h"

bool CapacitiveRead::read()
{
  static Timer timer(200);
  if (!timer.timePassed())
    return false;

  static int avg = touchRead(pin);
  static unsigned long lastTrue = 0;

  if (millis() - lastTrue > debounceMs && touchRead(pin) > (avg + (avg * deltaTreshold) / 100))
  {
    lastTrue = millis();
    // print("touch read true", pin, ">>", "avg =", avg, "val =", val, "treshold =", (avg + (avg * deltaTreshold) / 100));
    return true;
  }
  else
    return false;
}