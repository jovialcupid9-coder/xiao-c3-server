#include "prototypes.h"

namespace wire
{
    static TaskHandle_t taskHandle = NULL;

    void begin()
    {
        Wire.begin();
        Wire.setTimeout(1000);
    }

    static void scanTask(void *)
    {
        byte error;
        byte address;

        printer("Scanning I2C bus...");

        for (address = 1; address < 127; address++)
        {
            Wire.beginTransmission(address);
            error = Wire.endTransmission();

            if (error == 0)
                printer("Device found at address 0x", address < 16 ? "0" : "", (address, HEX));
            else if (error == 4)
                printer("Unknown error at address 0x", address < 16 ? "0" : "", (address, HEX));
        }

        printer("Scan completed");
        taskHandle = NULL; // not done automatically
        vTaskDelete(NULL);
    }

    void scan() // handles creating task and checks if it doesn't block 
    {
        while (taskHandle != NULL) // prevent multiple calls
            delay(10); 

        xTaskCreate(scanTask, NULL, 4096, NULL, 3, &taskHandle);

        Timer timer(I2C_FALLBACK_MS, true);
        while (!timer.timePassed())
        {
            if (taskHandle == NULL)
                return;
            else
                delay(10);
        }

        if (taskHandle != NULL) /* it should be already changed from task */
        {
            printer("timeout");
            vTaskDelete(taskHandle);
            taskHandle = NULL;
        }
    }
}
