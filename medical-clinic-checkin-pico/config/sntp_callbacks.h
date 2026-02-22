#ifndef SNTP_CALLBACKS_H
#define SNTP_CALLBACKS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void sntp_set_system_time(uint32_t sec, uint32_t us);

#ifdef __cplusplus
}
#endif

#endif
