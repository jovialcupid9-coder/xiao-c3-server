#include "prototypes.h"

static unsigned long simpleHash(const String &buff)
{
      uint32_t hash = 0;

      for (size_t i = 0; i < buff.length(); i++)
            hash = (hash * 31) + buff[i];

      return hash;
}

namespace patternMenager
{
      Pattern_t *listPtr[maxPatterns]{};
      Pattern_t pattern_1;
      Pattern_t pattern_2;
      Pattern_t pattern_3;
      Pattern_t pattern_4;
      Pattern_t pattern_5;

      Pattern_t::Pattern_t() : timer(this->interval)
      {
            int irr = 0;

            for (auto &p : listPtr)
            {
                  if (!p)
                  {
                        p = this;
                        id = "pattern-" + (String)irr;
                        return;
                  }

                  irr++;
            }

            printer("Pattern_t listPtr overflow!");
      }

      Pattern_t *dyspatch(String _id)
      { // to enable dynamic calling thru console using id

            for (auto *pattern : listPtr)
                  if (pattern && pattern->id == _id)
                        return pattern;

            printer("Pattern_t::dyspatch err for:", _id);
            delay(100);
            return nullptr;
      }

      void loop()
      {
            static Timer timer(10); // values above 10 doesn't change much

            if (timer.timePassed())
                  for (auto *pattern : listPtr)
                        if (pattern)
                              pattern->execute();
      }

      void stopAll()
      {
            printer("Stop all patterns");

            for (auto *p : listPtr)
                  if (p)
                        p->stop();
      }

      void startAll()
      {
            printer("Start all patterns");

            for (auto *p : listPtr)
                  if (p)
                        p->start();
      }

      bool Pattern_t::execute(const int overdrivePWM)
      {
            if (!running)
                  return false;

            if (!pin)
            {
                  logSteps = storage::get(id + "-logSteps", true);
                  pin = storage::get(id + "-pin");

                  if (!pin)
                  {
                        printer(id, "-pin not set");
                        running = false;
                        return false;
                  }

                  if (!isValidPWM(pin))
                  {
                        printer(id, "not valid pin for pwm:", pin);
                        pin = 0;
                        running = false;
                        return false;
                  }
            }
            
            static int lastHz = -1;
            if (lastHz != pwmHz)
                  if (ledcSetup(pwmChannel, constrain(pwmHz, 10, 20'000), pwmResolution))
                  {
                        ledcAttachPin(pin, pwmChannel);
                        printer("PWM setup OK: Channel", pwmChannel, "pin", pin, "freq", pwmHz, "res", pwmResolution);
                        lastHz = pwmHz;
                  }
                  else
                  {
                        printer("PWM setup ERR: Channel", pwmChannel, "pin", pin, "freq", pwmHz, "res", pwmResolution);
                        pin = 0;
                        running = false;
                        return false;
                  }

            if (!timer.timePassed())
                  return true;

            int pwm = pattern[step] + (pattern[step] * powerFactor) / 100;
            pwm = constrain(pwm, 0, pwmMaxValue);

            if (overdrivePWM != -1) // to 
                  pwm = overdrivePWM;
            
            if (logSteps)
                  printer(id, "on", pin, "step", step, "=",
                          "pwm:", normalizeLenght(pwm, 3),
                          "PF:", powerFactor,
                          "Hz:", pwmHz,
                          "ms:", interval,
                          "res:", pwmResolution);

            ledcWrite(pwmChannel, map(pwm, 0, 255, 0, pwmMaxValue));

            step++;
            if (step == len || pattern[step] == Pattern_t::terminator)
                  if (repeatPattern)
                        step = 0;
                  else
                        stop();

            return true;
      }

      bool Pattern_t::updatePattern(AsyncWebServerRequest *request)
      {
            return 0;
      }

      void Pattern_t::stop()
      {
            this->timer.push(); // to push NOW
            this->execute(0);
            running = false;
      }

      void Pattern_t::start()
      {
            if (this->pattern[0] == Pattern_t::terminator) // prevent start of empty ones
                  return;

            running = true;
            this->timer.push();
            this->execute();
      }

      void Pattern_t::changeInterval(const unsigned long ms)
      {
            if (ms > 20'000)
                  printer(id, "changeInterval range err for:", ms);
            else
            {
                  interval = ms;
                  timer.setInterval(interval);
            }
      }

}
