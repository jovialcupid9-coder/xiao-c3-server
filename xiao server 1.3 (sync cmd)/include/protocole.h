#pragma once
#include "prototypes.h"

#define PATTERN_LEN 999

namespace protocol
{
      extern int16_t cmd;

      static constexpr int terminator = -1;
      static const String argSeparator = " ";
      static const String objSeparator = ", ";

      // Strinify array that breaks using custom terminator
      // adding default len enables calling this funciton for both heap and stack stored patterns
      template <typename type, size_t size = PATTERN_LEN>
      static String stringifyPattern(const type (&arr)[size])
      {
            String result = "";
            String reserve(PATTERN_LEN * 4 + 4 /*termination*/);

            for (size_t i = 0; i < size; ++i)
            {
                  if (arr[i] == terminator)
                        break;

                  result += static_cast<String>(arr[i]);

                  if (i < size - 1)
                        result += argSeparator;
            }
            return "pattern = { " + result + " }";
      }
     

      void extractRecord(String &data, int &value);
};