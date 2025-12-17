#include "prototypes.h"

#include <Adafruit_BMP085.h>

/***************************************************
  This is an example for the BMP085 Barometric Pressure & Temp Sensor

  Designed specifically to work with the Adafruit BMP085 Breakout
  ----> https://www.adafruit.com/products/391

  These pressure and temperature sensors use I2C to communicate, 2 pins
  are required to interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/

// Connect VCC of the BMP085 sensor to 3.3V (NOT 5.0V!)
// Connect GND to Ground
// Connect SCL to i2c clock - on '168/'328 Arduino Uno/Duemilanove/etc thats Analog 5
// Connect SDA to i2c data - on '168/'328 Arduino Uno/Duemilanove/etc thats Analog 4
// EOC is not used, it signifies an end of conversion
// XCLR is a reset pin, also not used here

namespace arousalDetector
{
    constexpr auto tableLen = 2000; // 4kb
    unsigned short table[tableLen] = {};
    static auto tableIndex = 0;

    void update()
    {
        static int hz = storage::get("arousalDetector::hz", 1);

        static Timer timer(1000 / hz, true);
        if (!timer.timePassed())
            return;

        table[tableIndex] = pressureSensor::read();
        tableIndex++;
        if (tableIndex >= tableLen)
            tableIndex = 0;
    }

    void detectedEdge()
    {
        printer("close to edge, denial");
    }
}
namespace pressureSensor
{
    static Adafruit_BMP085 bmp;
    static bool working = false;

    bool fullReport()
    {
        if (!working)
            return working;

        printer(bmp.readTemperature(), "C,", bmp.readPressure(), "pa");
        printer("Raw altitude = ", bmp.readAltitude(), "m");
        // Calculate altitude assuming 'standard' barometric
        // pressure of 1013.25 millibar = 101325 Pascal
        printer("Corrected altitude = ", bmp.readAltitude(101500), "m");
        printer("Pressure at sealevel (calculated)", bmp.readSealevelPressure(), "pa");

        return false;
    }

    int read()
    {

        if (!working)
            working = bmp.begin(BMP085_STANDARD /* STANDARD IS NOT DEFAULT VALUE*/);

        return working ? bmp.readPressure() : false;

        /* i should add check if default value is not returned*/
    }
}
