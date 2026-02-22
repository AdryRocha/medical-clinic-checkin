#include "services/network_sync_service.hpp"
#include "services/data_storage_service.hpp"
#include "config/wifi_config.hpp"
#include "config/api_config.hpp"
#include "FreeRTOS.h"
#include "task.h"
#include <cJSON.h>
#include <cstdio>
#include <cstring>

NetworkSyncService& NetworkSyncService::getInstance() {
    static NetworkSyncService instance;
    return instance;
}

NetworkSyncService::NetworkSyncService() : access_token_("") {
}

NetworkSyncService::~NetworkSyncService() {
}

void NetworkSyncService::httpCallback(struct mg_connection *c, int ev, void *ev_data) {
    auto* ctx = static_cast<RequestContext*>(c->fn_data);
    
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = static_cast<struct mg_http_message*>(ev_data);
        int status = mg_http_status(hm);
        
        if (status == 200) {
            ctx->response->assign(hm->body.buf, hm->body.len);
            ctx->success = true;
        } else {
            printf("[NetworkSync] ERROR: HTTP status %d\n", status);
            ctx->success = false;
        }
        
        ctx->completed = true;
        c->is_closing = 1;
    }
    else if (ev == MG_EV_ERROR) {
        printf("[NetworkSync] ERROR: %s\n", (char*)ev_data);
        ctx->success = false;
        ctx->completed = true;
        c->is_closing = 1;
    }
}

// ============================================================================
// FUNÇÃO DE AUTENTICAÇÃO ATUALIZADA (cJSON + Host Dinâmico)
// ============================================================================
bool NetworkSyncService::authenticate(uint32_t timeout_ms) {
    char url[256];
    snprintf(url, sizeof(url), "%s%s", API_BASE_URL, API_ENDPOINT_AUTH);
    
    char body[256];
    snprintf(body, sizeof(body), 
             "{\"username\":\"%s\",\"password\":\"%s\"}",
             API_DEVICE_USERNAME, API_DEVICE_PASSWORD);
    
    std::string response;
    RequestContext ctx;
    ctx.response = &response;
    ctx.completed = false;
    ctx.success = false;
    
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    
    struct mg_connection *c = mg_http_connect(&mgr, url, httpCallback, &ctx);
    
    if (!c) {
        printf("[NetworkSync] ERROR: Falha ao criar conexao com o servidor\n");
        mg_mgr_free(&mgr);
        return false;
    }
    
    c->fn_data = &ctx;
    
    // Extrai o IP e porta real para o cabeçalho Host (Ex: 192.168.0.13:8000)
    const char* host_start = strstr(API_BASE_URL, "://");
    host_start = host_start ? host_start + 3 : API_BASE_URL;
    
    mg_printf(c,
              "POST %s HTTP/1.1\r\n"
              "Host: %s\r\n"
              "Content-Type: application/json\r\n"
              "Content-Length: %d\r\n"
              "\r\n"
              "%s",
              API_ENDPOINT_AUTH,
              host_start, 
              (int)strlen(body),
              body);
    
    uint32_t start_ms = to_ms_since_boot(get_absolute_time());
    
    while (!ctx.completed) {
        mg_mgr_poll(&mgr, 100);
        if (to_ms_since_boot(get_absolute_time()) - start_ms > timeout_ms) {
            printf("[NetworkSync] ERROR: Timeout na autenticacao!\n");
            ctx.success = false;
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    
    mg_mgr_free(&mgr);
    
    if (!ctx.success) {
        printf("[NetworkSync] Falha. O servidor respondeu com erro ou recusou: %s\n", response.c_str());
        return false;
    }
    
    // Parser seguro de JSON
    cJSON* root = cJSON_Parse(response.c_str());
    if (!root) {
        printf("[NetworkSync] ERROR: Resposta invalida (Nao eh JSON) -> %s\n", response.c_str());
        return false;
    }

    cJSON* token_json = cJSON_GetObjectItem(root, "access_token");
    if (token_json && cJSON_IsString(token_json)) {
        access_token_ = token_json->valuestring;
        cJSON_Delete(root);
        printf("[NetworkSync] Autenticado com SUCESSO!\n");
        return true;
    }
    
    cJSON_Delete(root);
    printf("[NetworkSync] ERROR: access_token ausente no JSON -> %s\n", response.c_str());
    return false;
}
// ============================================================================

bool NetworkSyncService::downloadAppointments(const std::string& date, 
                                              std::string& json_response,
                                              uint32_t timeout_ms) {
    if (access_token_.empty()) {
        printf("[NetworkSync] No token available, authenticating first...\n");
        if (!authenticate(API_AUTH_TIMEOUT_MS)) {
            printf("[NetworkSync] ERROR: Failed to authenticate\n");
            return false;
        }
    }
    
    char url[256];
    snprintf(url, sizeof(url), "%s%s?data=%s", 
             API_BASE_URL, API_ENDPOINT_APPOINTMENTS, date.c_str());
    
    RequestContext ctx;
    ctx.response = &json_response;
    ctx.completed = false;
    ctx.success = false;
    
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    
    struct mg_connection *c = mg_http_connect(&mgr, url, httpCallback, &ctx);
    
    if (!c) {
        printf("[NetworkSync] ERROR: Failed to create connection\n");
        mg_mgr_free(&mgr);
        return false;
    }
    
    c->fn_data = &ctx;
    
    char request_path[128];
    snprintf(request_path, sizeof(request_path), "%s?data=%s", 
             API_ENDPOINT_APPOINTMENTS, date.c_str());
    
    mg_printf(c,
              "GET %s HTTP/1.1\r\n"
              "Host: %s\r\n"
              "Authorization: Bearer %s\r\n"
              "Connection: close\r\n"
              "\r\n",
              request_path,
              "api-server",
              access_token_.c_str());
    
    uint32_t start_ms = to_ms_since_boot(get_absolute_time());
    
    while (!ctx.completed) {
        mg_mgr_poll(&mgr, 100);
        
        uint32_t elapsed_ms = to_ms_since_boot(get_absolute_time()) - start_ms;
        if (elapsed_ms > timeout_ms) {
            printf("[NetworkSync] ERROR: Timeout after %lu ms\n", elapsed_ms);
            ctx.success = false;
            break;
        }
        
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    
    mg_mgr_free(&mgr);
    
    if (!ctx.success) {
        printf("[NetworkSync] ERROR: Download failed\n");
        access_token_.clear();
    }
    
    return ctx.success;
}

bool NetworkSyncService::updateAppointmentStatus(int appointment_id, 
                                                  const char* new_status,
                                                  uint32_t timeout_ms) {
    if (access_token_.empty()) {
        printf("[NetworkSync] No token available, authenticating first...\n");
        if (!authenticate(API_AUTH_TIMEOUT_MS)) {
            printf("[NetworkSync] ERROR: Failed to authenticate\n");
            return false;
        }
    }
    
    char url[256];
    snprintf(url, sizeof(url), "%s/consultas/%d/status", 
             API_BASE_URL, appointment_id);
    
    std::string response;
    RequestContext ctx;
    ctx.response = &response;
    ctx.completed = false;
    ctx.success = false;
    
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    
    struct mg_connection *c = mg_http_connect(&mgr, url, httpCallback, &ctx);
    
    if (!c) {
        printf("[NetworkSync] ERROR: Failed to create connection\n");
        mg_mgr_free(&mgr);
        return false;
    }
    
    c->fn_data = &ctx;
    
    char request_path[128];
    snprintf(request_path, sizeof(request_path), "/consultas/%d/status?novo_status=%s", 
             appointment_id, new_status);
    
    mg_printf(c,
              "PATCH %s HTTP/1.1\r\n"
              "Host: %s\r\n"
              "Authorization: Bearer %s\r\n"
              "Connection: close\r\n"
              "\r\n",
              request_path,
              "api-server",
              access_token_.c_str());
    
    uint32_t start_ms = to_ms_since_boot(get_absolute_time());
    
    while (!ctx.completed) {
        mg_mgr_poll(&mgr, 100);
        
        uint32_t elapsed_ms = to_ms_since_boot(get_absolute_time()) - start_ms;
        if (elapsed_ms > timeout_ms) {
            printf("[NetworkSync] ERROR: Timeout after %lu ms\n", elapsed_ms);
            ctx.success = false;
            break;
        }
        
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    
    mg_mgr_free(&mgr);
    
    if (ctx.success) {
        printf("[NetworkSync] Status updated (appt %d -> %s)\n", appointment_id, new_status);
    } else {
        printf("[NetworkSync] ERROR: Failed to update status\n");
        access_token_.clear();
    }
    
    return ctx.success;
}

bool NetworkSyncService::downloadFingerprintTemplate(int patient_id,
                                                      std::vector<uint8_t>& template_data,
                                                      uint32_t timeout_ms) {
    if (access_token_.empty()) {
        printf("[NetworkSync] ERROR: No authentication token for fingerprint download\n");
        return false;
    }
    
    char url[256];
    snprintf(url, sizeof(url), "%s/pacientes/%d/fingerprint", API_BASE_URL, patient_id);
    
    std::string response;
    RequestContext ctx;
    ctx.response = &response;
    ctx.completed = false;
    ctx.success = false;
    
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    
    struct mg_connection *c = mg_http_connect(&mgr, url, httpCallback, &ctx);
    if (!c) {
        printf("[NetworkSync] ERROR: Failed to create connection\n");
        mg_mgr_free(&mgr);
        return false;
    }
    
    c->fn_data = &ctx;
    
    mg_printf(c,
              "GET /pacientes/%d/fingerprint HTTP/1.1\r\n"
              "Host: %s\r\n"
              "Authorization: Bearer %s\r\n"
              "\r\n",
              patient_id,
              "api-server",
              access_token_.c_str());
    
    uint32_t start_ms = to_ms_since_boot(get_absolute_time());
    
    while (!ctx.completed) {
        mg_mgr_poll(&mgr, 100);
        
        uint32_t elapsed_ms = to_ms_since_boot(get_absolute_time()) - start_ms;
        if (elapsed_ms > timeout_ms) {
            printf("[NetworkSync] ERROR: Download timeout\n");
            break;
        }
        
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    
    mg_mgr_free(&mgr);
    
    if (ctx.success && !response.empty()) {
        template_data.assign(response.begin(), response.end());
        return true;
    }
    
    printf("[NetworkSync] ERROR: Failed to download fingerprint for patient %d\n", patient_id);
    return false;
}

bool NetworkSyncService::uploadFingerprintTemplate(int patient_id,
                                                    const std::vector<uint8_t>& template_data,
                                                    uint32_t timeout_ms) {
    if (access_token_.empty()) {
        printf("[NetworkSync] ERROR: No authentication token for fingerprint upload\n");
        return false;
    }
    
    if (template_data.empty()) {
        printf("[NetworkSync] ERROR: Empty template data\n");
        return false;
    }
    
    char url[256];
    snprintf(url, sizeof(url), "%s/pacientes/%d/upload-fingerprint", API_BASE_URL, patient_id);
    
    std::string response;
    RequestContext ctx;
    ctx.response = &response;
    ctx.completed = false;
    ctx.success = false;
    
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    
    struct mg_connection *c = mg_http_connect(&mgr, url, httpCallback, &ctx);
    if (!c) {
        printf("[NetworkSync] ERROR: Failed to create connection\n");
        mg_mgr_free(&mgr);
        return false;
    }
    
    c->fn_data = &ctx;
    
    const char* boundary = "----PicoFingerprintBoundary";
    char filename[64];
    snprintf(filename, sizeof(filename), "fingerprint_%d.dat", patient_id);
    
    std::string http_request;
    
    char headers[512];
    snprintf(headers, sizeof(headers),
             "POST /pacientes/%d/upload-fingerprint HTTP/1.1\r\n"
             "Host: api-server\r\n"
             "Authorization: Bearer %s\r\n"
             "Content-Type: multipart/form-data; boundary=%s\r\n"
             "Content-Length: ",
             patient_id,
             access_token_.c_str(),
             boundary);
    
    http_request = headers;
    
    size_t multipart_size = 
        2 + strlen(boundary) + 2 +  // --boundary\r\n
        strlen("Content-Disposition: form-data; name=\"file\"; filename=\"") + strlen(filename) + 3 + // header line\r\n
        strlen("Content-Type: application/octet-stream\r\n") + 2 + // \r\n
        template_data.size() + 2 + // data\r\n
        2 + strlen(boundary) + 4;  // --boundary--\r\n
    
    char content_length[32];
    snprintf(content_length, sizeof(content_length), "%zu\r\nConnection: close\r\n\r\n", multipart_size);
    http_request += content_length;
    
    http_request += "--";
    http_request += boundary;
    http_request += "\r\n";
    http_request += "Content-Disposition: form-data; name=\"file\"; filename=\"";
    http_request += filename;
    http_request += "\"\r\n";
    http_request += "Content-Type: application/octet-stream\r\n\r\n";
    
    http_request.append(reinterpret_cast<const char*>(template_data.data()), template_data.size());
    
    http_request += "\r\n--";
    http_request += boundary;
    http_request += "--\r\n";
    
    mg_send(c, http_request.data(), http_request.size());
    
    uint32_t start_ms = to_ms_since_boot(get_absolute_time());
    
    while (!ctx.completed) {
        mg_mgr_poll(&mgr, 100);
        
        uint32_t elapsed_ms = to_ms_since_boot(get_absolute_time()) - start_ms;
        if (elapsed_ms > timeout_ms) {
            printf("[NetworkSync] ERROR: Upload timeout\n");
            break;
        }
        
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    
    mg_mgr_free(&mgr);
    
    if (ctx.success) {
        printf("[NetworkSync] Fingerprint uploaded (patient %d)\n", patient_id);
        return true;
    }
    
    printf("[NetworkSync] ERROR: Failed to upload fingerprint for patient %d\n", patient_id);
    return false;
}

int NetworkSyncService::downloadFingerprintsForAppointments(const std::string& json_appointments) {
    cJSON* root = cJSON_Parse(json_appointments.c_str());
    if (!root) {
        printf("[NetworkSync] ERROR: Failed to parse appointments JSON\n");
        return 0;
    }
    
    if (!cJSON_IsArray(root)) {
        printf("[NetworkSync] ERROR: JSON is not an array\n");
        cJSON_Delete(root);
        return 0;
    }
    
    int downloaded_count = 0;
    int array_size = cJSON_GetArraySize(root);
    auto& storage = DataStorageService::getInstance();
    
    for (int i = 0; i < array_size; i++) {
        cJSON* appointment = cJSON_GetArrayItem(root, i);
        if (!appointment) continue;
        
        cJSON* paciente = cJSON_GetObjectItem(appointment, "paciente");
        if (!paciente) continue;
        
        cJSON* aceita_digital = cJSON_GetObjectItem(paciente, "aceita_digital");
        cJSON* fingerprint_uploaded = cJSON_GetObjectItem(paciente, "fingerprint_uploaded");
        cJSON* patient_id_json = cJSON_GetObjectItem(paciente, "id");
        
        if (!aceita_digital || !fingerprint_uploaded || !patient_id_json) continue;
        
        bool accepts = cJSON_IsTrue(aceita_digital);
        bool has_fingerprint = cJSON_IsTrue(fingerprint_uploaded);
        int patient_id = patient_id_json->valueint;
        
        if (accepts && has_fingerprint) {
            if (storage.hasFingerprintTemplate(patient_id)) {
                continue;
            }
            
            std::vector<uint8_t> template_data;
            if (downloadFingerprintTemplate(patient_id, template_data)) {
                if (storage.saveFingerprintTemplate(patient_id, template_data)) {
                    downloaded_count++;
                } else {
                    printf("[NetworkSync] ERROR: Failed to save fingerprint for patient %d\n", patient_id);
                }
            }
            
            vTaskDelay(pdMS_TO_TICKS(200));
        }
    }
    
    cJSON_Delete(root);
    
    printf("[NetworkSync] Fingerprints downloaded: %d\n", downloaded_count);
    
    return downloaded_count;
}