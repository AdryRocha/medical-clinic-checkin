#ifndef FINGERPRINT_UPLOAD_SCREEN_HPP
#define FINGERPRINT_UPLOAD_SCREEN_HPP

#include <lvgl.h>

void fingerprint_upload_screen_init();
void fingerprint_upload_screen_show();
void fingerprint_upload_screen_hide();
void fingerprint_upload_screen_update_status(const char* message);

#endif