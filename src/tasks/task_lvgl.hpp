#ifndef TASK_LVGL_HPP
#define TASK_LVGL_HPP

#include <FreeRTOS.h>
#include <task.h>

/**
 * @brief LVGL task - handles UI updates
 * Must be called periodically to process LVGL events
 * @param params Not used
 */
void task_lvgl(void* params);

#endif // TASK_LVGL_HPP