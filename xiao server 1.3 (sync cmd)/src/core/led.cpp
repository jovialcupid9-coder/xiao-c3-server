#include "prototypes.h"

namespace led
{
  void write(unsigned char R, unsigned char G, unsigned char B, int brightness)
  {
    static int pin = storage::get("led_pin");
    if (!pin)
      printerInfo("pin not set");

#ifdef BoardRGB_swapColors
    neopixelWrite(pin, (G * brightness) / 255, (R * brightness) / 255, (B * brightness) / 255);
#else
    neopixelWrite(21, (R * brightness) / 255, (G * brightness) / 255, (B * brightness) / 255);
#endif
  }

}