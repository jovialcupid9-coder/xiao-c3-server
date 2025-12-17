#include "Timer.h"
//#define TIMER_LOG true

bool Timer::timePassed() 
{
	if( (millis() - timeStamp) >= interval){	
		timeStamp = millis();
		return true;
	}
      else
		return false;
} 

void Timer::setInterval(const long val) {
	if (val < 1) {
		Serial.println("timer::setInterval ill formated:" + (String)val);
		return;
	}
#ifdef TIMER_LOG
	Serial.println("timer::intervalChanged from " + (String)interval + "to " + (String)val);
#endif
	    interval = val;
}

const unsigned long Timer::getInterval() const { 
	return interval;
}

void Timer::push() { 
  timeStamp = 0;
}
