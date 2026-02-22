#include "services/appointment_service.hpp"
#include "services/data_storage_service.hpp"
#include "services/network_sync_service.hpp"
#include "services/time_service.hpp"
#include <cJSON.h>
#include <cstdio>
#include <cstring>
#include <cctype>

AppointmentService& AppointmentService::getInstance() {
    static AppointmentService instance;
    return instance;
}

AppointmentService::AppointmentService() {
}

AppointmentService::~AppointmentService() {
}

std::string AppointmentService::cleanCPF(const std::string& cpf) {
    std::string cleaned;
    cleaned.reserve(11);
    
    for (char c : cpf) {
        if (isdigit(c)) {
            cleaned += c;
        }
    }
    
    return cleaned;
}

std::string AppointmentService::maskCPF(const std::string& cpf) {
    if (cpf.length() >= 4) {
        return cpf.substr(0, 4) + "*******";
    }
    return "***********";
}

bool AppointmentService::validateAndParseQRCode(const std::string& qr_json, QRCodeData& qr_data) {
    cJSON* root = cJSON_Parse(qr_json.c_str());
    if (!root) {
        qr_data.valid = false;
        return false;
    }
    
    cJSON* cmd_json = cJSON_GetObjectItem(root, "cmd");
    cJSON* appt_id_json = cJSON_GetObjectItem(root, "appt_id");
    cJSON* cpf_json = cJSON_GetObjectItem(root, "cpf");
    cJSON* name_json = cJSON_GetObjectItem(root, "name");
    cJSON* hash_json = cJSON_GetObjectItem(root, "hash");
    
    if (!cmd_json || !cJSON_IsString(cmd_json) ||
        !appt_id_json || !cJSON_IsNumber(appt_id_json) ||
        !cpf_json || !cJSON_IsString(cpf_json) ||
        !name_json || !cJSON_IsString(name_json) ||
        !hash_json || !cJSON_IsString(hash_json)) {
        cJSON_Delete(root);
        qr_data.valid = false;
        return false;
    }
    
    qr_data.cmd = cmd_json->valuestring;
    qr_data.appt_id = appt_id_json->valueint;
    qr_data.cpf = cpf_json->valuestring;
    qr_data.name = name_json->valuestring;
    qr_data.hash = hash_json->valuestring;
    qr_data.valid = true;
    
    cJSON_Delete(root);
    
    if (qr_data.cmd != "checkin") {
        qr_data.valid = false;
        return false;
    }
    
    printf("[Appointment] QR parsed - ID: %d, Name: %s\n", qr_data.appt_id, qr_data.name.c_str());
    return true;
}

bool AppointmentService::validateAppointmentTime(const std::string& appointment_time) {
    int appt_hour, appt_min;
    if (!TimeService::extractHourMinute(appointment_time, appt_hour, appt_min)) {
        return false;
    }
    
    auto& timeService = TimeService::getInstance();
    std::string current_time_str = timeService.getTimeString();
    
    int current_hour, current_min;
    if (!TimeService::extractHourMinute(current_time_str, current_hour, current_min)) {
        return false;
    }
    
    int current_minutes = current_hour * 60 + current_min;
    int appt_minutes = appt_hour * 60 + appt_min;
    int diff_minutes = current_minutes - appt_minutes;
    
    return (diff_minutes >= -20 && diff_minutes <= 10);
}

bool AppointmentService::validateCPFMatch(const std::string& qr_cpf, const std::string& db_cpf) {
    std::string cleaned_qr = cleanCPF(qr_cpf);
    std::string cleaned_db = cleanCPF(db_cpf);
    return (cleaned_db == cleaned_qr);
}

bool AppointmentService::findAppointmentById(cJSON* root, int appt_id, cJSON** appointment_out) {
    if (!cJSON_IsArray(root)) {
        return false;
    }
    
    int array_size = cJSON_GetArraySize(root);
    
    for (int i = 0; i < array_size; i++) {
        cJSON* appointment = cJSON_GetArrayItem(root, i);
        if (!appointment) continue;
        
        cJSON* id_json = cJSON_GetObjectItem(appointment, "id");
        if (!id_json || !cJSON_IsNumber(id_json)) continue;
        
        if (id_json->valueint == appt_id) {
            *appointment_out = appointment;
            return true;
        }
    }
    
    return false;
}

void AppointmentService::extractAppointmentData(cJSON* appointment, AppointmentInfo& info) {
    cJSON* paciente_json = cJSON_GetObjectItem(appointment, "paciente");
    if (paciente_json && cJSON_IsObject(paciente_json)) {
        cJSON* cpf_json = cJSON_GetObjectItem(paciente_json, "cpf");
        if (cpf_json && cJSON_IsString(cpf_json)) {
            info.patient_cpf = cleanCPF(cpf_json->valuestring);
        }
        
        cJSON* name_json = cJSON_GetObjectItem(paciente_json, "nome");
        if (name_json && cJSON_IsString(name_json)) {
            info.patient_name = name_json->valuestring;
        }
        
        cJSON* patient_id_json = cJSON_GetObjectItem(paciente_json, "id");
        if (patient_id_json && cJSON_IsNumber(patient_id_json)) {
            info.patient.id = patient_id_json->valueint;
        }
        
        cJSON* aceita_digital_json = cJSON_GetObjectItem(paciente_json, "aceita_digital");
        if (aceita_digital_json) {
            info.patient.aceita_digital = cJSON_IsTrue(aceita_digital_json);
        }
        
        cJSON* fingerprint_uploaded_json = cJSON_GetObjectItem(paciente_json, "fingerprint_uploaded");
        if (fingerprint_uploaded_json) {
            info.patient.fingerprint_uploaded = cJSON_IsTrue(fingerprint_uploaded_json);
        }
        
        if (info.patient.aceita_digital) {
            if (info.patient.fingerprint_uploaded) {
                info.requires_fingerprint_verification = true;
            } else {
                info.requires_fingerprint_enrollment = true;
            }
        }
    }
    
    cJSON* profissional_json = cJSON_GetObjectItem(appointment, "profissional");
    if (profissional_json && cJSON_IsObject(profissional_json)) {
        cJSON* prof_json = cJSON_GetObjectItem(profissional_json, "nome");
        if (prof_json && cJSON_IsString(prof_json)) {
            info.professional_name = prof_json->valuestring;
        }
        
        cJSON* categoria_json = cJSON_GetObjectItem(profissional_json, "categoria");
        if (categoria_json && cJSON_IsObject(categoria_json)) {
            cJSON* cat_json = cJSON_GetObjectItem(categoria_json, "nome");
            if (cat_json && cJSON_IsString(cat_json)) {
                info.category = cat_json->valuestring;
            }
        }
    }
    
    cJSON* time_json = cJSON_GetObjectItem(appointment, "horario");
    if (time_json && cJSON_IsString(time_json)) {
        info.time = time_json->valuestring;
    }
    
    cJSON* status_json = cJSON_GetObjectItem(appointment, "status");
    if (status_json && cJSON_IsString(status_json)) {
        info.status = status_json->valuestring;
    }
    
    cJSON* hash_json = cJSON_GetObjectItem(appointment, "qr_code_hash");
    if (hash_json && cJSON_IsString(hash_json)) {
        info.qr_hash = hash_json->valuestring;
    }
    
    cJSON* id_json = cJSON_GetObjectItem(appointment, "id");
    if (id_json && cJSON_IsNumber(id_json)) {
        info.id = id_json->valueint;
    }
}

bool AppointmentService::validateAppointmentById(const QRCodeData& qr_data, AppointmentInfo& info) {
    if (!qr_data.valid) {
        info.found = false;
        return false;
    }
    
    std::string cleaned_cpf = cleanCPF(qr_data.cpf);
    
    if (cleaned_cpf.length() != 11) {
        info.found = false;
        return false;
    }
    
    auto& dataStorage = DataStorageService::getInstance();
    std::string today = dataStorage.getTodayDate();
    
    if (today.empty()) {
        info.found = false;
        return false;
    }
    
    std::string json_data;
    if (!dataStorage.readAppointments(today, json_data)) {
        info.found = false;
        return false;
    }
    
    cJSON* root = cJSON_Parse(json_data.c_str());
    if (!root) {
        info.found = false;
        return false;
    }
    
    cJSON* appointment = nullptr;
    if (!findAppointmentById(root, qr_data.appt_id, &appointment)) {
        cJSON_Delete(root);
        info.found = false;
        return false;
    }
    
    extractAppointmentData(appointment, info);
    info.found = true;
    
    if (!validateCPFMatch(cleaned_cpf, info.patient_cpf)) {
        cJSON_Delete(root);
        info.found = false;
        return false;
    }
    
    if (info.status == "realizada") {
        cJSON_Delete(root);
        return false;
    }
    
    if (!info.qr_hash.empty() && info.qr_hash != qr_data.hash) {
        printf("[AppointmentService] Hash mismatch! QR=%s, DB=%s\n",
               qr_data.hash.c_str(), info.qr_hash.c_str());
    }
    
    if (!validateAppointmentTime(info.time)) {
        cJSON_Delete(root);
        info.found = false;
        return false;
    }
    
    cJSON_Delete(root);
    printf("[Appointment] Validated: %s at %s\n", info.patient_name.c_str(), info.time.c_str());
    return true;
}

bool AppointmentService::markAppointmentCompleted(int appointment_id) {
    auto& dataStorage = DataStorageService::getInstance();
    std::string today = dataStorage.getTodayDate();
    
    if (today.empty()) {
        return false;
    }
    
    if (!dataStorage.markAppointmentCompleted(today, appointment_id)) {
        return false;
    }
    
    auto& networkSync = NetworkSyncService::getInstance();
    networkSync.updateAppointmentStatus(appointment_id, "realizada");
    
    return true;
}