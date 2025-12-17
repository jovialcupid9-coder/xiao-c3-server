#pragma once
#include <Arduino.h>

///////////////////////////////////////  SUGAR SYNTAX MACROS  /////////////////////////////////////////////////////////////////

typedef void (*funcPtr)(); // syntax: void f(funcPtr func){ func(); }

#define STR_HELPER(x) #x     // stringifies literal
#define STR(x) STR_HELPER(x) // forces expansion

#define func(...) [](__VA_ARGS__)
#define for_x(limit) for (auto x = 0; x <= limit; x++)
#define forArr(arr) for (auto x = 0; sizeof(arr) / sizeof(arr[0]); x++)


static constexpr const char* stripPath(const char* path)  // Strings are only run time
{
    const char* p = path;
    for(const char* s = path; *s != '\0'; ++s)
        if(*s == '/' || *s == '\\') p = s + 1;
    return p;
}

#define printer(...) printer_imp("[" + (String)stripPath(__FILE__) + "::" + __FUNCTION__ + ":" + __LINE__ + "]", __VA_ARGS__)
#define printerInfo(...) if (printInfo) printer("[I]", __VA_ARGS__)

template<typename T>
inline String qt(T val) {
    return "'" + (String)val + "'";
}


// official one sucks, it takes val two times, more overhead, can break logic
template<typename T>
inline T clamp(T val, T low, T high)
{
    T t = val < low ? low : val> high ? high : val;

    if(t != val) 
        printer("clamped val", val, "to", t);

    return t;
}

template <typename F>
inline void measureTime(const String &label, F function)
{
    unsigned long startTime = micros();
    function();
    unsigned long duration = micros() - startTime;

    printer(
        label, ":",
        duration < 1000 ? duration: (float)duration / 1000,
        duration < 1000 ? "µs" : "ms"
            );
}

/////////////////////////////////////////  BUILT-IN / EXTERNAL LIBS  /////////////////////////////////////////////////////////////////

#include <WiFi.h>
#include <ESPmDNS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSerial.h>
#include <FS.h>
#include <SPIFFS.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>
#include <esp_log.h>
#include <ArduinoJson.h>

//////////////////////////////////////////////  INTERNAL LIBS  //////////////////////////////////////////////////////////////////////////

#include "Timer.h"


///////////////////////////////////////////////////  SRC  //////////////////////////////////////////////////////////////////////////////////////////

#include "led.h"
#include "wire.h"
#include "myPrint.h"
#include "ezButton.h"
#include "console.h"
#include "storage.h"
#include "protocole.h"
#include "network.h"
#include "patterns.h"
#include "powerbank.h"
#include "performance.h"
#include "pressureSensor.h"


///////////////////////////////////////////////  PROTOTYPES  ////////////////////////////////////////////////////////////////

bool isValidPWM(const byte pin);
bool isSafe(const byte pin);


