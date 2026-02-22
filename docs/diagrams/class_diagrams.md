# Class Diagrams

Diagramas de classes de todas as camadas do sistema.

---

## 1. Services Layer

Camada de serviços com lógica de negócio.

```mermaid
classDiagram
    direction TB

    class StateMachine {
        <<Singleton>>
        -State state_
        +getInstance() StateMachine&
        +setState(State newState) void
        +getState() State
        +getStateName() const char*
        +getStateName(State state) const char*
    }

    class State {
        <<enumeration>>
        INITIALIZING
        DOWNLOADING_APPOINTMENTS
        IDLE
        VALIDATING
        FINGERPRINT_VERIFYING
        FINGERPRINT_ENROLLING
        FINGERPRINT_UPLOADING
        APPOINTMENT
        ERROR
        ERROR_CRITICAL
        RESTARTING
    }

    class AppointmentService {
        <<Singleton>>
        +getInstance() AppointmentService&
        +validateAndParseQRCode(qr_json, qr_data) bool
        +validateAppointmentById(qr_data, info) bool
        +markAppointmentCompleted(appointment_id) bool
        +maskCPF(cpf) string$
        -findAppointmentById(root, appt_id, appointment_out) bool
        -extractAppointmentData(appointment, info) void
        -validateCPFMatch(qr_cpf, db_cpf) bool
        -validateAppointmentTime(appointment_time) bool
        -cleanCPF(cpf) string
    }

    class FingerprintService {
        <<Singleton>>
        -R307S_Driver* fp_driver_
        -HAL_UART_RP2040* uart_hal_
        -bool is_initialized_
        +getInstance() FingerprintService&
        +init() bool
        +isInitialized() bool
        +enrollFingerprint(template_data, timeout_per_scan_ms) bool
        +verifyFingerprint(patient_id, confidence, timeout_ms) bool
        +clearSensorMemory() bool
        +getTemplateCount(count) bool
        -loadTemplateToSensor(data, slot) bool
        -waitForFinger(timeout_ms) bool
        -waitForFingerRemoval(timeout_ms) bool
    }

    class NetworkSyncService {
        <<Singleton>>
        -string access_token_
        +getInstance() NetworkSyncService&
        +authenticate(timeout_ms) bool
        +downloadAppointments(date, json_response, timeout_ms) bool
        +updateAppointmentStatus(appointment_id, new_status, timeout_ms) bool
        +downloadFingerprintTemplate(patient_id, template_data, timeout_ms) bool
        +uploadFingerprintTemplate(patient_id, template_data, timeout_ms) bool
        +downloadFingerprintsForAppointments(json_appointments) int
        +hasToken() bool
        -httpCallback(c, ev, ev_data) void$
    }

    class DataStorageService {
        <<Singleton>>
        +getInstance() DataStorageService&
        +init() bool
        +saveAppointments(date, json_data) bool
        +hasAppointments(date) bool
        +readAppointments(date, json_data) bool
        +getTodayDate() string
        +getTomorrowDate() string
        +isReady() bool
        +markAppointmentCompleted(date, appointment_id) bool
        +isAppointmentCompleted(date, appointment_id) bool
        +saveFingerprintTemplate(patient_id, template_data) bool
        +loadFingerprintTemplate(patient_id, template_data) bool
        +hasFingerprintTemplate(patient_id) bool
        +deleteFingerprintTemplate(patient_id) bool
        +clearAllFingerprints() bool
    }

    class TimeService {
        <<Singleton>>
        -bool initialized_
        -bool time_synced_
        +getInstance() TimeService&
        +init() bool
        +getTimeString() string
        +getDateString() string
        +getDateTimeString() string
        +isTimeSynced() bool
        +markTimeSynced() void
        +extractHourMinute(time_str, hour, min) bool$
    }

    class AppointmentInfo {
        <<struct>>
        +int id
        +string patient_name
        +string patient_cpf
        +string professional_name
        +string category
        +string time
        +string status
        +string qr_hash
        +bool found
        +PatientBiometry patient
        +bool requires_fingerprint_verification
        +bool requires_fingerprint_enrollment
    }

    class PatientBiometry {
        <<struct>>
        +int id
        +bool aceita_digital
        +bool fingerprint_uploaded
    }

    class QRCodeData {
        <<struct>>
        +string cmd
        +int appt_id
        +string cpf
        +string name
        +string hash
        +bool valid
    }

    %% Relacionamentos
    StateMachine *-- State : contains
    AppointmentInfo *-- PatientBiometry : contains

    AppointmentService ..> QRCodeData : uses
    AppointmentService ..> AppointmentInfo : uses
    AppointmentService ..> DataStorageService : depends
    AppointmentService ..> NetworkSyncService : depends
    AppointmentService ..> TimeService : depends

    FingerprintService ..> DataStorageService : loads templates
    FingerprintService ..> R307S_Driver : controls sensor

    NetworkSyncService ..> DataStorageService : saves to

    DataStorageService ..> TimeService : gets date
```

---

## 2. HAL Layer (Hardware Abstraction)

Interfaces abstratas e implementações específicas do RP2040.

```mermaid
classDiagram
    direction TB

    class HAL_SPI_Interface {
        <<interface>>
        +init(baudrate) bool*
        +write(data, len) size_t*
        +read(data, len) size_t*
        +setCS(state) void*
        +setDC(state) void*
        +reset() void*
    }

    class HAL_I2C_Interface {
        <<interface>>
        +init(baudrate) bool*
        +write(address, data, length, sendStop) bool*
        +read(address, data, length, sendStop) bool*
        +writeRead(address, writeData, writeLength, readData, readLength) bool*
    }

    class HAL_UART_Interface {
        <<interface>>
        +init(baudrate, data_bits, stop_bits, parity) bool*
        +write(data, length) size_t*
        +read(data, length) size_t*
        +available() size_t*
        +writeByte(byte) void*
        +readByte() uint8_t*
    }

    class HAL_SPI_RP2040 {
        -spi_inst_t* spi_instance_
        -uint8_t pin_mosi_
        -uint8_t pin_miso_
        -uint8_t pin_sck_
        -uint8_t pin_cs_
        -uint8_t pin_dc_
        -uint8_t pin_rst_
        +init(baudrate) bool
        +write(data, len) size_t
        +read(data, len) size_t
        +setCS(state) void
        +setDC(state) void
        +reset() void
    }

    class HAL_I2C_RP2040 {
        -i2c_inst_t* i2c_instance_
        -uint8_t pin_sda_
        -uint8_t pin_scl_
        +init(baudrate) bool
        +write(address, data, length, sendStop) bool
        +read(address, data, length, sendStop) bool
        +writeRead(...) bool
    }

    class HAL_UART_RP2040 {
        -uart_inst_t* uart_instance_
        -uint8_t pin_tx_
        -uint8_t pin_rx_
        +init(baudrate, data_bits, stop_bits, parity) bool
        +write(data, length) size_t
        +read(data, length) size_t
        +available() size_t
    }

    HAL_SPI_Interface <|.. HAL_SPI_RP2040 : implements
    HAL_I2C_Interface <|.. HAL_I2C_RP2040 : implements
    HAL_UART_Interface <|.. HAL_UART_RP2040 : implements
```

---

## 3. Driver Layer

Drivers de hardware com interfaces abstratas.

```mermaid
classDiagram
    direction TB

    class DisplayInterface {
        <<interface>>
        +init() bool*
        +getWidth() uint16_t*
        +getHeight() uint16_t*
        +drawPixels(x1, y1, x2, y2, color_data) void*
        +fillRect(x1, y1, x2, y2, color) void*
        +setBacklight(brightness) void*
        +setPower(state) void*
        +setRotation(rotation) void*
    }

    class TouchInterface {
        <<interface>>
        +init() bool*
        +readTouch(point) bool*
        +isTouched() bool*
        +getMaxTouchPoints() uint8_t*
    }

    class QR_Interface {
        <<interface>>
        +init() bool*
        +readScan(buffer, max_length, timeout_ms) size_t*
        +isScanAvailable() bool*
        +setScanCallback(callback) void*
        +process() void*
        +enableScan(enable) bool*
        +triggerScan() bool*
    }

    class FingerprintInterface {
        <<interface>>
        +init() bool*
        +verifyPassword() bool*
        +getImage() FingerprintStatus*
        +image2Tz(slot) FingerprintStatus*
        +createModel() FingerprintStatus*
        +storeModel(id) FingerprintStatus*
        +fingerFastSearch(match) FingerprintStatus*
        +deleteModel(id) FingerprintStatus*
        +emptyDatabase() FingerprintStatus*
        +getTemplateCount(count) FingerprintStatus*
        +enrollFingerprint(id) FingerprintStatus*
        +matchFingerprint(match) FingerprintStatus*
        +loadTemplate(id, slot) FingerprintStatus*
        +uploadTemplate(slot, data) FingerprintStatus*
        +downloadTemplate(slot, data) FingerprintStatus*
        +compareTemplates(confidence) FingerprintStatus*
        +setLED(on) bool*
    }

    class ST7796Driver {
        -HAL_SPI_Interface* hal_spi_
        -uint16_t width_
        -uint16_t height_
        -uint8_t rotation_
        +init() bool
        +drawPixels(...) void
        +fillRect(...) void
        +setRotation(rotation) void
    }

    class FT6336U_Driver {
        -HAL_I2C_Interface* i2c_
        -uint8_t i2c_address_
        -uint16_t screen_width_
        -uint16_t screen_height_
        +init() bool
        +readTouch(point) bool
        +isTouched() bool
        +reset() void
    }

    class GM67_Driver {
        -HAL_UART_Interface* uart_hal_
        -ScanCallback scan_callback_
        -char rx_buffer_[1024]
        +init() bool
        +readScan(buffer, max_length, timeout_ms) size_t
        +isScanAvailable() bool
        +process() void
        +sendCommand(command) bool
    }

    class R307S_Driver {
        -HAL_UART_Interface* uart_hal_
        -uint32_t password_
        -uint32_t address_
        -bool is_initialized_
        -size_t data_packet_size_
        +init() bool
        +verifyPassword() bool
        +getImage() FingerprintStatus
        +image2Tz(slot) FingerprintStatus
        +createModel() FingerprintStatus
        +storeModel(id) FingerprintStatus
        +fingerFastSearch(match) FingerprintStatus
        +deleteModel(id) FingerprintStatus
        +emptyDatabase() FingerprintStatus
        +getTemplateCount(count) FingerprintStatus
        +enrollFingerprint(id) FingerprintStatus
        +matchFingerprint(match) FingerprintStatus
        +loadTemplate(id, slot) FingerprintStatus
        +uploadTemplate(slot, data) FingerprintStatus
        +downloadTemplate(slot, data) FingerprintStatus
        +compareTemplates(confidence) FingerprintStatus
        +setLED(on) bool
        +readSysPara(status, system_id, library_size, security_level) FingerprintStatus
    }

    class TouchPoint {
        <<struct>>
        +uint16_t x
        +uint16_t y
        +uint8_t event
        +bool valid
    }

    class FingerprintStatus {
        <<enumeration>>
        OK
        ERROR_COMM
        ERROR_NO_FINGER
        ERROR_ENROLL_FAIL
        ERROR_BAD_IMAGE
        ERROR_TOO_MESSY
        ERROR_FEATURE_FAIL
        ERROR_NO_MATCH
        ERROR_NOT_FOUND
        ERROR_MERGE_FAIL
        ERROR_DELETE_FAIL
        ERROR_CLEAR_FAIL
        ERROR_WRONG_PASSWORD
        ERROR_NO_TEMPLATE
        ERROR_UPLOAD_FAIL
        ERROR_RECV_FAIL
        ERROR_TIMEOUT
    }

    class FingerprintMatch {
        <<struct>>
        +uint16_t id
        +uint16_t confidence
        +bool matched
    }

    DisplayInterface <|.. ST7796Driver : implements
    TouchInterface <|.. FT6336U_Driver : implements
    QR_Interface <|.. GM67_Driver : implements
    FingerprintInterface <|.. R307S_Driver : implements

    ST7796Driver --> HAL_SPI_Interface : uses
    FT6336U_Driver --> HAL_I2C_Interface : uses
    GM67_Driver --> HAL_UART_Interface : uses
    R307S_Driver --> HAL_UART_Interface : uses

    TouchInterface ..> TouchPoint : returns
    FingerprintInterface ..> FingerprintStatus : returns
    FingerprintInterface ..> FingerprintMatch : returns
```

---

## 4. Adapter Layer

Adaptadores entre LVGL e drivers de hardware.

```mermaid
classDiagram
    direction TB

    class LVGLDisplayAdapter {
        -DisplayInterface* display_
        -lv_disp_drv_t disp_drv_
        -lv_disp_draw_buf_t draw_buf_
        -lv_color_t* buf1_
        -lv_color_t* buf2_
        +LVGLDisplayAdapter(display, buf1, buf2, buf_size)
        +registerDisplay() lv_disp_t*
        -flushCallback(disp_drv, area, color_p) void$
    }

    class LVGLTouchAdapter {
        -TouchInterface* touch_
        -lv_indev_drv_t indev_drv_
        -lv_indev_t* indev_
        -uint16_t display_width_
        -uint16_t display_height_
        -uint8_t rotation_
        +LVGLTouchAdapter(touch, width, height, rotation)
        +registerInputDevice() lv_indev_t*
        -readCallback(drv, data) void$
        -transformCoordinates(point, x_out, y_out) void
    }

    class DisplayInterface {
        <<interface>>
    }

    class TouchInterface {
        <<interface>>
    }

    LVGLDisplayAdapter --> DisplayInterface : uses
    LVGLTouchAdapter --> TouchInterface : uses
```

---
