#include "performance.h"


namespace performance
{
  namespace frequency
  {
    bool logEachRead = false;
    static unsigned long ctrLast = 0;

    void loop()
    {
      static unsigned long ctr = 0;
      ctr++;

      static Timer timer(1000, true);
      if (!timer.timePassed())
        return;

      if (logEachRead || ctr < 5'000)
        printer(formatFreq(ctr));

      ctrLast = ctr;
      ctr = 0;
    }

    unsigned long getLastFreq(){
      return ctrLast;
    }

  }

  void heapLeft()
  {
    static Timer timer(500);
    if (!timer.timePassed())
      return;

    if (ESP.getFreeHeap() < 15'000)
      printer("HEAP WARNING!", ESP.getFreeHeap(), "b left");
   
      const static int threshold = storage::get("fragmentationWarning", 55);
    if (getFragmentation() > threshold)
      printer("fragmentation warning", getFragmentation(), "%!");

    if (ESP.getFreeHeap() < 5'000)
      console::execute("reset(heaplefterr)");
  }

  float getFragmentation()
  {
    return 100 - ESP.getMaxAllocHeap() * 100.0 / ESP.getFreeHeap();
  }
}
