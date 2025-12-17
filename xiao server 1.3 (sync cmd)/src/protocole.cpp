#include "prototypes.h"

namespace protocol
{
      void extractRecord(String &data, int &value)
      {
            const int separatorIndex = data.indexOf(argSeparator);

            if (separatorIndex == terminator)
            {
                  data = "";
                  value = terminator;
            }
            else
            {
                  value = data.substring(0, separatorIndex).toInt();
                  data = data.substring(separatorIndex + 1);
            }
      }
}