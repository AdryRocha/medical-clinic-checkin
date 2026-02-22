#include "hal/rp2350/hal_power_rp2350.hpp"
#include "pico/stdlib.h"
#include "hardware/watchdog.h"

void HAL_Power_RP2350::reboot() {
    watchdog_reboot(0, 0, 0);
    // Loop infinito para garantir que o processador não faça nada enquanto espera o reset
    while(1) {
        __asm volatile ("nop");
    }
}

void HAL_Power_RP2350::sleep() {
    // CORREÇÃO: Substituído __wfi() por assembly inline compatível com GCC/ARM
    __asm volatile ("wfi");
}