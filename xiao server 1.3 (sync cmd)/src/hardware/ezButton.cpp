
#include <ezButton.h>

ezButton::ezButton(const String id, const int mode)
{
	this->id = id;
	this->mode = mode;
	debounceTime = 50;
	count = 0;
	countMode = COUNT_FALLING;
	lastDebounceTime = 0;
	previousSteadyState = LOW;
	lastSteadyState = previousSteadyState;
	lastFlickerableState = previousSteadyState;
}

void ezButton::setDebounceTime(const unsigned long time)
{
	debounceTime = time;
}

int ezButton::getState()
{
	return lastSteadyState;
}

bool ezButton::isPressed()
{
	return (previousSteadyState == unpressedState && lastSteadyState == pressedState);
}

bool ezButton::isReleased()
{
	return (previousSteadyState == pressedState && lastSteadyState == unpressedState);
}

void ezButton::setCountMode(const int countMode)
{
	this->countMode = countMode;
}

void ezButton::onPress(void (*handler)())
{
	pressHandler = handler;
}

void ezButton::onRelease(void (*handler)())
{
	releaseHandler = handler;
}

static const String decodePinMode(const int mode)
{
	switch (mode)
	{
	case INTERNAL_PULLUP:
		return "INTERNAL_PULLUP";
		break;
	case INTERNAL_PULLDOWN:
		return "INTERNAL_PULLDOWN";
		break;
	case EXTERNAL_PULLDOWN:
		return "EXTERNAL_PULLDOWN";
		break;
	case EXTERNAL_PULLUP:
		return "EXTERNAL_PULLUP";
		break;

	default:
		return "UNKOWN";
		break;
	}
}

void ezButton::loop()
{
	static bool setup = true;
	static const int pin = storage::get(id + "-pin");
	static const int mode = storage::get(id + "-mode");

	if (!pin)
		return;

	if (setup)
	{
		setup = false;
		printer("installing new button on pin", pin, "with", decodePinMode(mode));

		if (mode == INTERNAL_PULLUP || mode == INTERNAL_PULLDOWN)
			pinMode(pin, mode);
		else if (mode == EXTERNAL_PULLUP || mode == EXTERNAL_PULLDOWN)
			pinMode(pin, INPUT); // External pull-up/pull-down, set as INPUT

		// Set the pressed and unpressed states based on the mode
		if (mode == INTERNAL_PULLDOWN || mode == EXTERNAL_PULLDOWN)
		{
			pressedState = HIGH;
			unpressedState = LOW;
		}
		else
		{
			pressedState = LOW;
			unpressedState = HIGH;
		}
	}

	int currentState = digitalRead(pin);
	unsigned long currentTime = millis();

	// check to see if you just pressed the button
	// (i.e. the input went from LOW to HIGH), and you've waited long enough 
	// since the last press to ignore any noise:

	// If the switch/button changed, due to noise or pressing:
	if (currentState != lastFlickerableState)
	{
		// reset the debouncing timer
		lastDebounceTime = currentTime;
		// save the the last flickerable state
		lastFlickerableState = currentState;
	}

	if ((currentTime - lastDebounceTime) >= debounceTime)
	{
		// whatever the reading is at, it's been there for longer than the debounce
		// delay, so take it as the actual current state:

		// save the the steady state
		previousSteadyState = lastSteadyState;
		lastSteadyState = currentState;
	}

	if (previousSteadyState != lastSteadyState)
	{
		if (countMode == COUNT_BOTH)
			count++;
		else if (countMode == COUNT_FALLING && previousSteadyState == HIGH && lastSteadyState == LOW)
			count++;
		else if (countMode == COUNT_RISING && previousSteadyState == LOW && lastSteadyState == HIGH)
			count++;
	}

	if (isPressed() && pressHandler)
	{
		printer(id, pin, "pressed");
		pressHandler();
	}

	else if (isReleased() && releaseHandler)
	{
		printer(id, pin, "released");
		releaseHandler();
	}
}
