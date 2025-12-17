#include "prototypes.h"

JsonDocument cfg;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* TODO::
repair pattern extraction

hide button mode into class itself

make /dashboard stop refreshing data as often when is not in focus
*/

ezButton button("button");
String button_mode = "";
 
static constexpr int safePins[] = {
#ifdef CONFIG_IDF_TARGET_ESP32C3
    2, 3, 4, 5, 6, 7, 8, 9, 10, 18, 19, 20, 21
#elif CONFIG_IDF_TARGET_ESP32S3
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14
#else
 #error "DEFINE SAFE TO USE PINS"
#endif
};

bool isValidPWM(const byte pin)
{

  static constexpr int validPWMpins[] = {
#ifdef CONFIG_IDF_TARGET_ESP32C3
      2,
      3,
      4,
      /* A3, gpio5 is making problems if using wifi*/ 6,
      7,
      8,
      9,
      10,
#elif CONFIG_IDF_TARGET_ESP32S3
      0,
      1,
      2,
      3,
      4,
      5,
      6,
      7,
      8,
      9,
#else
#endif
  };

  for (auto p : validPWMpins)
    if (p == pin)
      return true;

  return false;
};

void setup()
{
  for (auto pin : safePins)
  {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }

  storage::init();
  console::init();

  // esp_log_level_set("*", storage::get("loglvl", ESP_LOG_VERBOSE));

  wire::begin();

  button_mode = storage::get("button-mode", "HOLD");
  button.onPress([]()
                 {
                      if (button_mode.equalsIgnoreCase("HOLD"))
                        pattern_1.stop();

                      else if (button_mode.equalsIgnoreCase("CHANGE"))
                        pattern_1.running ? pattern_1.stop() : pattern_1.start(); });
  button.onRelease([]()
                   {
                      if (button_mode.equalsIgnoreCase("HOLD"))
                        pattern_1.start(); });

  network::begin();
  powerbank::begin();

  led::write(GREEN);
  printer("---- SETUP ENDED ----");
}

void loop()
{
  patternMenager::loop();

  console::loop();
  WebSerial.loop();

  performance::heapLeft();
  performance::frequency::loop();

  powerbank::controller();

  button.loop();
}
