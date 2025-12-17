#include "prototypes.h"

bool printToSerial = true;
bool printToServer = false;
bool printInfo = false;

String normalizeLenght(const int val, const int targetLen)
{
      String buff = (String)val;
      while (buff.length() < targetLen)
            buff += " ";

      return buff;
};

String stringify(bool b)
{
      return b ? "ok" : "err";
}
String stringify(IPAddress addr)
{
      return addr.toString();
}

String formatFreq(float hz)
{
      const char *units[] = {"Hz", "kHz", "MHz", "GHz"};
      int i = 0;

      while (hz >= 1000 && i < 3)
      {
            hz /= 1000;
            i++;
      }

      return stringify(hz, units[i]);
}
String formatBytes(double bytes)
{
      const char *units[] = {"B", "KB", "MB", "GB"}; // megabits (Mb) =/= megabyte (MB)
      int unitIndex = 0;

      while (bytes >= 1024 && unitIndex < 3)
      {
            bytes /= 1024;
            unitIndex++;
      }

      return stringify(bytes, units[unitIndex]);
}
String formatBytes(double used, double total)
{
      if (total == 0)
            return "unavailable";

      const char *units[] = {"B", "KB", "MB", "GB"};
      int unitIndex = 0;

      while (total >= 1024 && unitIndex < 3)
      {
            used /= 1024;
            total /= 1024;
            unitIndex++;
      }

      return stringify(used, "/", total, " ", units[unitIndex], " [", (used * 100.0 / total), "%]");
}
String formatMs(unsigned long ms)
{
      if (ms < 1000)
            return stringify(ms, "ms");

      const unsigned long seconds = ms / 1000;
      if (seconds < 60)
            return stringify(seconds, "s");

      const unsigned long minutes = seconds / 60;
      if (minutes < 60)
            return stringify(minutes, "m ", seconds % 60, "s");

      const unsigned long hours = minutes / 60;
      if (hours < 24)
            return stringify(hours, "h ", minutes % 60, "m ", seconds % 60, "s");

      const unsigned long days = hours / 24;
      return stringify(days, days == 1 ? "day " : "days ", hours % 24, "h ", minutes % 60, "m ", seconds % 60, "s");
}
