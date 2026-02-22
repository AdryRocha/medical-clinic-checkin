#include "hal/rp2350/hal_watchdog_rp2350.hpp"
#include "hardware/watchdog.h"

void HAL_Watchdog_RP2350::init(uint32_t timeout_ms) {
    // Inicia o watchdog. Se não for alimentado em timeout_ms, o chip reseta.
    watchdog_enable(timeout_ms, 1);
}

void HAL_Watchdog_RP2350::feed() {
    watchdog_update();
}

void HAL_Watchdog_RP2350::reboot() {
    // Força um reboot imediato configurando gatilho para 1ms
    watchdog_enable(1, 1);
    while(1); // Trava aqui até o hardware resetar
}