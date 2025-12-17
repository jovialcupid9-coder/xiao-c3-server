#pragma once
#include "prototypes.h"

#define COUNT_FALLING 0
#define COUNT_RISING  1
#define COUNT_BOTH    2

// Constants for button modes
#define INTERNAL_PULLUP  INPUT_PULLUP
#ifdef INPUT_PULLDOWN
#define INTERNAL_PULLDOWN INPUT_PULLDOWN
#else
#define INTERNAL_PULLDOWN INPUT
#endif

#define EXTERNAL_PULLUP   0xFE
#define EXTERNAL_PULLDOWN 0xFF

class ezButton
{
	private:
		int mode;
		unsigned long debounceTime;
		unsigned long count;
		int countMode;
		int pressedState;     // the state when the button is considered pressed
		int unpressedState;   // the state when the button is considered unpressed

		int previousSteadyState;  // the previous steady state from the input pin, used to detect pressed and released event
		int lastSteadyState;      // the last steady state from the input pin
		int lastFlickerableState; // the last flickerable state from the input pin

		unsigned long lastDebounceTime; // the last time the output pin was toggled
		void (*pressHandler)() = nullptr;
		void (*releaseHandler)() = nullptr;

	public:
	    String id;
		ezButton(const String id, const int mode = INTERNAL_PULLUP);
		void setDebounceTime(const unsigned long time);
		int getState();
		bool isPressed();
		bool isReleased();
		void onPress(void (*handler)());
		void onRelease(void (*handler)());
		void setCountMode(int mode);
		void loop();
};

