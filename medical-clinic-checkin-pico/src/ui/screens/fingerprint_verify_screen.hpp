#ifndef FINGERPRINT_OPERATION_SCREEN_HPP
#define FINGERPRINT_OPERATION_SCREEN_HPP

#include <lvgl.h>

enum FingerprintOperationMode {
    FINGERPRINT_VERIFY,
    FINGERPRINT_ENROLL
};

void fingerprint_operation_screen_init();
void fingerprint_operation_screen_show(FingerprintOperationMode mode);
void fingerprint_operation_screen_set_step(int step);  // Only for ENROLL mode
void fingerprint_operation_screen_update_status(const char* message);
void fingerprint_operation_screen_hide();

#endif