#ifndef APPOINTMENT_SCREEN_HPP
#define APPOINTMENT_SCREEN_HPP

void appointment_screen_show();
void appointment_screen_update(const char* patient_name,
                              const char* cpf,
                              const char* appointment_id,
                              const char* professional_info);

#endif // APPOINTMENT_SCREEN_HPP