#ifndef TASK_WIFI_TIME_INIT_HPP
#define TASK_WIFI_TIME_INIT_HPP

#include <FreeRTOS.h>
#include <event_groups.h>

extern EventGroupHandle_t wifi_time_ready_group_event;

#define WIFI_CONNECTED_BIT (1 << 0)
#define TIME_SYNCED_BIT (1 << 1)
#define LVGL_READY_BIT (1 << 2)

/**
 * @brief WiFi and time initialization task
 * 
 * This task handles:
 * - WiFi hardware initialization
 * - WiFi connection with retry logic
 * - Time service initialization (NTP sync)
 * - Signals when both WiFi and time are ready via event group
 * 
 * @param params Task parameters (unused)
 */
void task_wifi_time_init(void* params);

#endif // TASK_WIFI_TIME_INIT_HPP