#include "prototypes.h"

namespace storage
{
    static SemaphoreHandle_t mutex;
    static constexpr size_t MAX_PATH_LENGTH = 31;

    // Helper function to build file path, returns without changes if correct format was given
    static String validatePath(const String &key)
    {
        String path;
        path.reserve(MAX_PATH_LENGTH);

        if (!key.startsWith("/"))
            path = "/";

        path += key;

        if (path.indexOf(".") == -1) // assume ext if none is given
            path += ".txt";

        path.trim();
        path.toLowerCase();

         if (path.length() > MAX_PATH_LENGTH)
        {
            printer("Path too long:", path, "max:", MAX_PATH_LENGTH);
            return "";
        }
        return path;
    }

    static bool put_imp(const String &key, const String &content, const char * mode = "w")
    {
        String path = validatePath(key);
        if (path.isEmpty())
            return false;


        xSemaphoreTake(mutex, portMAX_DELAY);
        File file = SPIFFS.open(path, mode);
        if (!file)
        {
            printer("Failed to create:", path);
            file.close();
            xSemaphoreGive(mutex);
            return false;
        }

        const size_t written = file.print(content);
        file.close();
        xSemaphoreGive(mutex);

        bool res = (written == content.length());
        if (!res)
            printer("Write failed for:", path, "written:", written, "expected:", content.length());
        else
            printerInfo(path, "written", formatBytes(written));

        return res;
    }  

    static String read_imp(const String &key, const String& defaultValue)
    {
        String path = validatePath(key);
        if (path.isEmpty())
            return "";

         if (!exists(path))
         {
            printer(path, "doesn't exists creating with def val", qt(defaultValue));
            put_imp(path, defaultValue);
            return defaultValue;
         }
         
        xSemaphoreTake(mutex, portMAX_DELAY);
        File file = SPIFFS.open(path, "r");
        if (!file)
        {
            printer("Failed to open:", path);
            xSemaphoreGive(mutex);
            return String();
        }

        String content = file.readString();
        file.close();
        xSemaphoreGive(mutex);

        if(content.isEmpty())
        {
            printer(path, "exists empty, setting def val", qt(defaultValue));
            put_imp(path, defaultValue);
            return defaultValue;
        }
        //printInfo(path, "readed", qt(content));

        return content;
    }

    // ==================== PUBLIC FUNCTION IMPLEMENTATIONS ====================

    size_t getSize(const String &key)
    {
        String path = validatePath(key);
        if (path.isEmpty())
            return size_t(0);

        xSemaphoreTake(mutex, portMAX_DELAY);

        File file = SPIFFS.open(path, "r");
        if (!file)
        {
            printer("Failed to open:", path);
            xSemaphoreGive(mutex);
            return size_t(0);
        }
        size_t size = file.size();
        file.close();
        xSemaphoreGive(mutex);
        return size;
    }

    bool init()
    {
        if(!mutex)
            mutex = xSemaphoreCreateMutex();

        if (!SPIFFS.begin(true))
        {
            printer("Failed to mount SPIFFS");
            return false;
        }
        else
            return true;
    }

    bool exists(const String &key)
    {
        String path = validatePath(key);

        if (path.isEmpty())
            return false;

        xSemaphoreTake(mutex, portMAX_DELAY);
        bool res = SPIFFS.exists(path);
        xSemaphoreGive(mutex);

        if (!res)
            printer(path, "doesn't exist");

        return res;
    }

    bool remove(const String &key)
    {
        String path = validatePath(key);
        if (path.isEmpty())
            return false;

            xSemaphoreTake(mutex, portMAX_DELAY);
            bool res = SPIFFS.remove(path);
            xSemaphoreGive(mutex);
            printer("Remove", key, "=", res);
            return res;
    }

    bool format()
    {
        printer("Formatting storage...");
        xSemaphoreTake(mutex, portMAX_DELAY);
        bool res = SPIFFS.format();
        xSemaphoreGive(mutex);
        printer("Format =", res);
        return res;
    }

    String getKeywords(bool with_values)
    {
        String result;

        xSemaphoreTake(mutex, portMAX_DELAY);
        File root = SPIFFS.open("/");
        File file = root.openNextFile();

        while (file)
        {
            String filename = file.name();
            if (filename.endsWith(".txt"))
            {
                String key = filename.substring(0, filename.length() - 4); // Remove "/" and ".txt"

                if (key.length())
                {
                    result += key;

                    if (with_values)
                    {
                        result += " = ";
                        // Read value directly
                        file.seek(0);
                        String value;
                        while (file.available())
                            value += (char)file.read();

                        value.trim();
                        result += value;
                    }

                    result += "\n";
                }
            }
            file = root.openNextFile();
        }

        xSemaphoreGive(mutex);

        return result.length() ? result : "No keys found";
    }

    String getFiles(bool with_sizes)
    {
        String result;
        xSemaphoreTake(mutex, portMAX_DELAY);
        File root = SPIFFS.open("/");
        File file = root.openNextFile();

        while (file)
        {
            result += file.name();

            if (with_sizes)
            {
                result += " = ";
                result += formatBytes(file.size());
            }

            result += "\n";
            file = root.openNextFile();
        }
        xSemaphoreGive(mutex);
        return result.length() ? result : "No files found";
    }

    bool rename(const String &path, const String &path2)
    {
        xSemaphoreTake(mutex, portMAX_DELAY);
        bool res = SPIFFS.rename(path, path2);
        xSemaphoreGive(mutex);
        printerInfo("rename", path, path2, "=", res);
        return res;
    }
    
    // ==================== PUT TEMPLATE SPECIALIZATIONS ====================

    template <>
    bool put<int>(const String &key, int value, const char * mode)
    {
        return put_imp(key, (String)value, mode);
    }

    template <>
    bool put<bool>(const String &key, bool value, const char * mode)
    {
        return put_imp(key, (String)value, mode);
    }

    template <>
    bool put<float>(const String &key, float value, const char * mode)
    {
        return put_imp(key, String(value, 3), mode);
    }

    template <>
    bool put<double>(const String &key, double value, const char * mode)
    {
        return put_imp(key, String(value, 3), mode);
    }

    template <>
    bool put<String>(const String &key, String value, const char * mode)
    {
        return put_imp(key, value, mode);
    }

    template <>
    bool put<const char *>(const String &key, const char *value, const char * mode)
    {
        return put_imp(key, (String)value, mode);
    }

    template <>
    bool put<JsonDocument>(const String &key, JsonDocument json, const char * mode)
    {
        String out;
        serializeJson(json, out);
        return put_imp(key, out);
    }

    template <>
    bool put<esp_log_level_t>(const String &key, esp_log_level_t value, const char * mode)
    {
        return put_imp(key, String((int)value));
    }

    // ==================== GET WITH DEFAULT VALUE ====================

    template <>
    int get<int>(const String &key, int default_value)
    {
        return read_imp(key, (String)default_value).toInt();   
    }

    template <>
    bool get<bool>(const String &key, bool default_value)
    {
        String content = read_imp(key, (String)default_value);
 
        content.toLowerCase();
        return (content == "true" || content == "1");
    }

    template <>
    float get<float>(const String &key, float default_value)
    {
        return read_imp(key, (String)default_value).toFloat();
    }

    template <>
    double get<double>(const String &key, double default_value)
    {
        return read_imp(key, (String)default_value).toDouble();
    }

    template <>
    String get<String>(const String &key, String default_value)
    {
        return read_imp(key, default_value);
    }

    template <>
    JsonDocument get<JsonDocument>(const String &key, JsonDocument emptyJson)
    {
        String in;
        in = get<String>(key, String("{}"));
        JsonDocument json;
        deserializeJson(json, in);
        /// add error check
        return json;
    }

    template <>
    esp_log_level_t get<esp_log_level_t>(const String &key, esp_log_level_t default_value)
    {
        return (esp_log_level_t)constrain((long)read_imp(key, String(default_value)).toInt(), (long)ESP_LOG_NONE, (long)ESP_LOG_VERBOSE);
    }

    String get(const String &key, const char *default_value)
    {
        return read_imp(key, (String)default_value);
    }
    
    // ==================== GET PROXY IMPLEMENTATION ====================

    GetProxy::operator int() const
    {
        return get<int>(key_, 0);
    }

    GetProxy::operator bool() const
    {
        return get<bool>(key_, false);
    }

    GetProxy::operator float() const
    {
        return get<float>(key_, 0.0f);
    }

    GetProxy::operator double() const
    {
        return get<double>(key_, 0.0);
    }

    GetProxy::operator String() const
    {
        return get<String>(key_, "");
    }

    JsonDocument emptyJson;
    GetProxy::operator JsonDocument() const
    {
        return get<JsonDocument>(key_, emptyJson);
    }

    GetProxy::operator esp_log_level_t() const
    {
        return get<esp_log_level_t>(key_, ESP_LOG_DEBUG);
    }

    // Main get function (returns proxy)
    GetProxy get(const String &key)
    {
        return GetProxy(key);
    }


} // namespace storage
