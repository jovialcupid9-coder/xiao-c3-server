#pragma once
#include "prototypes.h"

#define cache(expr) [] { static auto _cached = (expr); return _cached; }()

/// ======= SYNTAX =======
//  f(get("key", 1))  OK
//  f(get<int>("key"))  OK
//  f(get("key"))  OK
//  int v = get("key", 1)  OK
//  int v = get<int>("key")  OK
//  int v = get("key")  OK
//  auto v = get("key", 1)  OK
//  auto v = get<int>("key") OK
//  auto v = get("key") ERR


namespace storage
{
    bool init();
    bool exists(const String &key);
    bool remove(const String &key);
    bool format();
    bool rename(const String &path, const String &path2);
    String getKeywords(bool with_values = true);
    String getFiles(bool with_sizes = true);
    size_t getSize(const String &key);

    template <typename T>
    bool put(const String &key, T value, const char * mode ="w");

    // GET with default - deduces type from default value
    template <typename T>
    T get(const String &key, T defaultValue);

    String get(const String &key, const char *defaultValue); // overload needed for String default parameter, vsc treats is as const char *

    // GET without default - returns proxy object that converts to target type
    class GetProxy
    {
    private:
        String key_;

    public:
        explicit GetProxy(const String &key) : key_(key) {}

        // Conversion operators
        operator int() const;
        operator bool() const;
        operator float() const;
        operator double() const;
        operator String() const;
        operator JsonDocument() const;
        operator esp_log_level_t() const;

        // Template conversion for other types
        template <typename T>
        operator T() const
        {
            printer("unknown type, returning def");
            return T{};
        }
    };

    // Main get function - returns proxy for type deduction
    GetProxy get(const String &key);

} // namespace storage
