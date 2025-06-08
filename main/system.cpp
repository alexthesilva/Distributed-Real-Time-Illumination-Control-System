#include "system.h"
#include "pico/time.h"

System::System() {
    startTime = to_ms_since_boot(get_absolute_time());
}

float System::getElapsedSeconds() const {
    uint32_t currentTime = to_ms_since_boot(get_absolute_time());
    uint32_t elapsedMillis = currentTime - startTime;
    return elapsedMillis / 1000.0f;
}

void System::doReset() const {
    watchdog_reboot(0, 0, 0);
    while (true) { }
}
