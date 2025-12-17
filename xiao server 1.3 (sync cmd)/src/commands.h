#pragma once

// custom print function that do not put function name inside, it returns operator()
#ifdef printer
#undef printer
#define printer(...) printer_imp("[" + (String)stripPath(__FILE__) + ":" + __LINE__ + "]", __VA_ARGS__)
#endif

static void reset(String reason)
{
    reason.trim();

    if (reason.isEmpty())
        reason = "console";

    printer("board will reset by", qt(reason)), "\n\n";
    delay(1000); // time needed for webserial

    led::write(OFF); // software reboot doesn't turn off diode

    ESP.restart();
}

console::Cmd console::cmd_list[] = 
{
    {"sysInfo", []()
     {
         printer("uploaded: " __DATE__ " " __TIME__);
         printer("up time:", formatMs(millis()));
         printer("chip model:", ESP.getChipModel(), "revision:", ESP.getChipRevision());
         printer("sdk version:", ESP.getSdkVersion());
         printer("cpu freq:", String(getCpuFrequencyMhz()) + "Mhz");
         printer("task freq:", formatFreq(performance::frequency::getLastFreq())); 
         printer("heap:", formatBytes(ESP.getFreeHeap(), ESP.getHeapSize()));
         printer("fragmentation:", String(performance::getFragmentation()) + "%");
         printer("psram:", formatBytes(ESP.getMaxAllocPsram(), ESP.getPsramSize()));
         printer("sketch:", formatBytes(ESP.getSketchSize(), ESP.getFreeSketchSpace() + ESP.getSketchSize()));
         printer("spiffs:", formatBytes(SPIFFS.usedBytes(), SPIFFS.totalBytes()));
    
     }},

    {"digitalWrite", "pin val", []()
     {
         int pin = extractInt();
         pinMode(pin, OUTPUT);
         digitalWrite(pin, extractBool());
     }},
    {"digitalRead", "pin mode", []()
     {
         int pin = extractInt();
         auto mode = extractString().equalsIgnoreCase("pulldown") ? INPUT_PULLDOWN : INPUT_PULLUP;
         pinMode(pin, mode);
         delay(5); // really needed!
         printer(digitalRead(pin));
     }},

    {"analogRead", "pin", []()
     {
         printer(analogRead(extractInt()));
     }},
    {"analogWrite", "pin val", []()
     {
         analogWrite(extractInt(), extractInt());
     }},

    {"restart", []()
     {
         reset(extractString());
     }},
    {"reset", []()
     {
         reset(extractString());
     }},

    {"help", []()
     {
         printCmds();
     }},

    {"setCpuFreq", "Mhz", []()
     {
         int newHz = extractInt();

         constexpr int validFreqs[] = {
             240, 160, 80, // For all types of XTAL crystal
             40, 20, 10,   // For 40MHz XTAL
             26, 13,       // For 26MHz XTAL
             24, 12        // For 24MHz XTAL
         };

         for (const auto hz : validFreqs)
             if (hz == newHz)
             {
                 printer("setCpuFreq to", newHz + "Hz", setCpuFrequencyMhz((uint32_t)newHz));
                 return;
             }
         printer("setCpuFreq range err, hz cannot be set to", newHz);
     }},
    {"getCpuFrequencyMhz", []()
     {
         printer(getCpuFrequencyMhz(), "MHz");
     }},

    {"esp_log_level_set", "tag level", []()
     {
         esp_log_level_set(extractString().c_str(), esp_log_level_t(extractInt()));
         ESP_LOGI("commands", "hello");
         ESP_LOGD("commands", "hello");
         ESP_LOGV("commands", "hello");
     }},
    {"esp_log_level_get", "tag", []()
     {
         printer(esp_log_level_get(extractString().c_str()));
     }},

    {"wifi::IP", []()
     {
         printer(network::wifi::getIP());
     }},
    {"wifi::getCreditenstialsList", []()
     {
         printer(network::wifi::getCreditenstialsList());
     }},
    {"wifi::connect", "ssid password", []()
     {
         network::wifi::connect(extractString(), extractString());
     }},
    {"wifi::getSSID", []()
     {
         printer(network::wifi::getSSID());
     }},
    {"wifi::getRSSI", []()
     {
         printer(network::wifi::getRSSI());
     }},
    {"wifi::getScan", []()
     {
         printer(network::wifi::getScan());
     }},
    {"wifi::getHostName", []()
     {
         printer(qt(WiFi.getHostname()));
     }},

    {"storage::get", "key def", []()
     {
        const String key = extractString();
        const String def = extractString();
        if(def.isEmpty())
            printer((storage::get(key)));
        else 
            printer(storage::get(key, def));
     }},
    {"storage::put", "key val", []()
     {
         printer(storage::put(extractString(), extractString()));
     }},
    {"storage::remove", "key", []()
     {
         printer(storage::remove(extractString()));
     }},
    {"storage::exists", "key", []()
     {
         printer(storage::exists(extractString()));
     }},
    {"storage::format", []()
     {
         printer(storage::format());
     }},
    {"storage::rename", "path newPath", []()
     {
        storage::rename(extractString(), extractString());
     }},
   {"formatMs", "time", []()
     {
       printer(formatMs(extractInt()));
     }},
    {"storage::getKeywords", "withVal = false", []()
     {
         bool withValues = true;

         const String arg = extractString();
         if (arg != "")
             withValues = arg.toInt();

         printer('\n', storage::getKeywords(withValues));
     }},
    {"storage::getFiles", "withSize = false", []()
     {
         bool withSize = true;

         const String arg = extractString();
         if (arg != "")
             withSize = arg.toInt();

         printer('\n', storage::getFiles(withSize));
     }},

    {"powerbank::turnOff", []()
     {
         powerbank::turnOff();
     }},
    {"powerbank::percentCharged", []()
     {
         printer(powerbank::percentCharged(), "%");
     }},
    {"powerbank::readRawVoltage", []()
     {
         printer(powerbank::readRawVoltage(), "V");
     }},

    {"pressureSensor::read", []()
     {
         printer(pressureSensor::read(), "Pa");
     }},

    {"frequency::logEachRead", "bool", []()
     {
         performance::frequency::logEachRead = extractBool();
     }},

    {"wire::scan", []()
     {
         wire::scan();
     }},
    {"upTime", []()
     {
         printer(millis());
     }},

    {"testJson", []()
     {
         JsonDocument doc;
         JsonArray data = doc["data"].to<JsonArray>();

         JsonObject item1 = data.add<JsonObject>();
         String key = "hostname";
         item1["dataname"] = key;
         item1["current_value"] = storage::get(key, WiFi.getHostname());
         item1["input_type"] = "text";
         item1["max_len"] = 20;

         JsonObject item2 = data.add<JsonObject>();
         key = "WiFi SSID";
         item2["dataname"] = key;
         item2["current_value"] = storage::get(key, "");
         item2["input_type"] = "text";
         item2["max_len"] = 30;

         JsonObject item3 = data.add<JsonObject>();
         key = "printToSerial";
         item3["dataname"] = key;
         item3["current_value"] = storage::get(key, true);
         item3["input_type"] = "boolean";

         storage::put("config.json", doc);
     }},

    {"led::write", "R G B brightness", []()
     {
         led::write(extractInt(), extractInt(), extractInt(), extractInt());
     }},

    {"Pattern_t::newPattern", "whichLine", []()
     {
         patternMenager::Pattern_t *obj = patternMenager::dyspatch(extractString());

         if (obj)
             obj->updatePattern();
     }},
    {"loadPattern", "id whichLine", []()
     {
         String id = extractString();
         String savedPath = extractString();

         File file = SPIFFS.open(savedPath, "r");
         if (!file)
             return;

         auto obj = patternMenager::dyspatch(id);

         for (auto &step : obj->pattern)
             step = file.read();

         for (int step = 0; step < Pattern_t::len; step++)
             printer(step, ":", obj->pattern[step]);

         obj->step = 0;
         obj->running = true;
     }} // do  put trailing comma, it breaks code... even it should work from 99'

};
