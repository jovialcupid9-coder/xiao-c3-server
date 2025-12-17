#pragma once
#include "prototypes.h"
/* must be compiled with at least gnu++17 */

extern bool printToSerial;
extern bool printToServer;
extern bool printInfo;

template <typename... Args>
String stringify(const Args &...args)
{
    return (... + (static_cast<String>(args)));
}
String stringify(bool b); 
String stringify(IPAddress addr);

template <typename... Args>
void printer_imp(const Args &...args)
{
    static SemaphoreHandle_t printMutex = NULL;
    if (!printMutex)
        printMutex = xSemaphoreCreateMutex();

    static String buff;
    buff.reserve(2048);

    ((buff += (String)args + String(" ")), ...); // must be buffored for webSerial

    xSemaphoreTake(printMutex, portMAX_DELAY);

    if (printToSerial)
        Serial.println(buff);
    if (printToServer)
        WebSerial.println(buff);

    xSemaphoreGive(printMutex);

    buff.clear();
}


String normalizeLenght(const int val, const int targetLen);
String formatBytes(double bytes);
String formatBytes(double used, double total);
String formatMs(unsigned long ms);
String formatFreq(float Hz);
