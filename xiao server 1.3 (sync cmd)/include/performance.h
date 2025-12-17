#pragma once
#include "prototypes.h"

namespace performance
{
 float getFragmentation();

  void heapLeft();

  namespace frequency {
    extern bool logEachRead;
    void loop();
    unsigned long getLastFreq();
  }
  
}
