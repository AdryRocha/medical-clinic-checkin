#pragma once

// CORREÇÃO: O include precisa do caminho completo a partir da pasta src
#include "hal/interfaces/hal_wifi_interface.hpp"

#ifdef __cplusplus
extern "C" {
#endif

// Cria o ícone (chamado uma vez no boot)
void ui_create_wifi_icon();

// Atualiza o status (chamado pela task de WiFi)
void ui_set_wifi_status(WiFiStatus status);

#ifdef __cplusplus
}
#endif