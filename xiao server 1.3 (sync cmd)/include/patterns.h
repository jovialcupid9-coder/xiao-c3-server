#pragma once
#include "Arduino.h"

namespace patternMenager
{
      class Pattern_t
      {
      public:
            Pattern_t();

            String id; // cannot be const reference
            int pin = 0;

            static constexpr int len = 999;
            static constexpr int terminator = -1;
            int pattern[len]{}; /* int to enforce terminator */
            
            bool logSteps = true;
            bool repeatPattern = true;

            int step = 0;
            int pwmHz = 100; 
            int powerFactor = 0;
            bool running = 0;
            Timer timer;
            bool execute(const int overdrivePWM = -1);
            bool updatePattern(AsyncWebServerRequest *request = NULL);
            void stop();
            void start();
            void changeInterval(unsigned long ms);
         

      private:
            int interval = 1000; //ms
            
            bool pwmInitialized = false;
            int pwmChannel = 0;
            int pwmResolution = 12;
            int pwmMaxValue = (1 << pwmResolution) - 1; // Start with 8-bit
      };
      extern Pattern_t pattern_1;
      extern Pattern_t pattern_2;
      extern Pattern_t pattern_3;
      extern Pattern_t pattern_4;
      extern Pattern_t pattern_5;
      
      constexpr int maxPatterns = 5;
      extern Pattern_t *listPtr[maxPatterns];

      Pattern_t *dyspatch(String id);
      void loop();
      void stopAll();
      void startAll();
}

using namespace patternMenager;