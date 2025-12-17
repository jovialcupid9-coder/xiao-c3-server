#pragma once
#include "Arduino.h"

#define TIMER_LIB_VERSION  4

#define NON_BLOCKING_DELAY(x) { static Timer localTimer(x); if (!localTimer.timepassed()) return; }

class Timer 
{
  unsigned long interval = 0; 
  unsigned long timeStamp = 0;

public:
  Timer(unsigned long interval, bool startRightAway = false) : interval(interval) 
  {
    timeStamp = millis() + (startRightAway ? 0 : interval);
  };

  bool timePassed();
  void push();
  void setInterval(const long);
  const unsigned long getInterval() const;
};
