#pragma once
#include "prototypes.h"

namespace wire
{
    constexpr unsigned long I2C_FALLBACK_MS = 10'000;
    
    void begin();
    void scan();
}
