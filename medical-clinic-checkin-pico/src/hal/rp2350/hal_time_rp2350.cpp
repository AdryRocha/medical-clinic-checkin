#include "hal/rp2350/hal_time_rp2350.hpp"

#include "pico/time.h"
#include "pico/stdlib.h"

uint64_t HAL_Time_RP2350::millis() {
    return to_ms_since_boot(get_absolute_time());
}

uint64_t HAL_Time_RP2350::micros() {
    return time_us_64();
}

void HAL_Time_RP2350::delay_ms(uint32_t ms) {
    sleep_ms(ms);
}
