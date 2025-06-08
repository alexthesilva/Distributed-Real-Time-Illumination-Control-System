#ifndef SYSTEM_H
#define SYSTEM_H

#include <Arduino.h>
#include "pico/stdlib.h"
#include "hardware/watchdog.h"

class System {
public:
    System();
    float getElapsedSeconds() const;
    void doReset() const;
private:
    uint32_t startTime;
};

#endif // SYSTEM_H
