#include "r307s_driver.hpp"
#include "pico/stdlib.h"
#include <cstring>
#include <cstdio>

R307S_Driver::R307S_Driver(HAL_UART_Interface* uart_hal, uint32_t password, uint32_t address)
    : uart_hal_(uart_hal),
      password_(password),
      address_(address),
      is_initialized_(false),
      data_packet_size_(128) {
    memset(packet_buffer_, 0, PACKET_BUFFER_SIZE);
}

bool R307S_Driver::init() {
    if (uart_hal_ == nullptr) {
        return false;
    }

    // Clear any pending data in UART buffer
    uart_hal_->clearRxBuffer();
    
    // Small delay for sensor to stabilize after power-on
    sleep_ms(500);
    
    // Verify connection with the sensor
    if (!verifyPassword()) {
        return false;
    }

    is_initialized_ = true;

    // Read sensor system parameters to get data packet size
    uint16_t status_reg, sys_id, lib_size, sec_level;
    FingerprintStatus sys_status = readSysPara(status_reg, sys_id, lib_size, sec_level);
    if (sys_status == FingerprintStatus::OK) {
        // Data packet size is at offset 12-13 in system parameters
        // In packet_buffer_: [0-1]=start, [2-5]=addr, [6]=PID, [7-8]=len, [9]=confirm,
        //   [10-11]=status_reg, [12-13]=sys_id, [14-15]=lib_size, [16-17]=sec_level,
        //   [18-21]=device_addr, [22-23]=data_packet_size_code, [24-25]=baud_rate
        uint16_t pkt_size_code = readU16(&packet_buffer_[22]);
        switch (pkt_size_code) {
            case 0: data_packet_size_ = 32; break;
            case 1: data_packet_size_ = 64; break;
            case 2: data_packet_size_ = 128; break;
            case 3: data_packet_size_ = 256; break;
            default: data_packet_size_ = 128; break;
        }
        printf("[R307S] Sensor packet size: %zu bytes (code=%d), library: %d, security: %d\n",
               data_packet_size_, pkt_size_code, lib_size, sec_level);
    } else {
        printf("[R307S] WARNING: Could not read system params, using default packet size 128\n");
        data_packet_size_ = 128;
    }

    return true;
}

bool R307S_Driver::verifyPassword() {
    uint8_t data[4];
    writeU32(data, password_);
    
    if (!sendPacket(CMD_VERIFY_PASSWORD, data, 4)) {
        return false;
    }

    if (receivePacket() == 0) {
        return false;
    }

    return getConfirmationCode() == static_cast<uint8_t>(FingerprintStatus::OK);
}

FingerprintStatus R307S_Driver::getImage() {
    if (!is_initialized_) {
        return FingerprintStatus::ERROR_COMM;
    }

    if (!sendPacket(CMD_GEN_IMAGE)) {
        return FingerprintStatus::ERROR_COMM;
    }

    if (receivePacket(IMAGE_TIMEOUT_MS) == 0) {
        return FingerprintStatus::ERROR_TIMEOUT;
    }

    return static_cast<FingerprintStatus>(getConfirmationCode());
}

FingerprintStatus R307S_Driver::image2Tz(uint8_t slot) {
    if (!is_initialized_) {
        return FingerprintStatus::ERROR_COMM;
    }

    if (slot != 1 && slot != 2) {
        return FingerprintStatus::ERROR_BAD_LOCATION;
    }

    uint8_t data[1] = { slot };
    
    if (!sendPacket(CMD_IMAGE_2_TZ, data, 1)) {
        return FingerprintStatus::ERROR_COMM;
    }

    if (receivePacket(DEFAULT_TIMEOUT_MS * 2) == 0) {
        return FingerprintStatus::ERROR_TIMEOUT;
    }

    return static_cast<FingerprintStatus>(getConfirmationCode());
}

FingerprintStatus R307S_Driver::createModel() {
    if (!is_initialized_) {
        return FingerprintStatus::ERROR_COMM;
    }

    if (!sendPacket(CMD_REG_MODEL)) {
        return FingerprintStatus::ERROR_COMM;
    }

    if (receivePacket() == 0) {
        return FingerprintStatus::ERROR_TIMEOUT;
    }

    return static_cast<FingerprintStatus>(getConfirmationCode());
}

FingerprintStatus R307S_Driver::storeModel(uint16_t id) {
    if (!is_initialized_) {
        return FingerprintStatus::ERROR_COMM;
    }

    uint8_t data[3];
    data[0] = 0x01;  // Buffer ID: 0x01 = CharBuffer1, 0x02 = CharBuffer2
    writeU16(&data[1], id);
    
    if (!sendPacket(CMD_STORE, data, 3)) {
        return FingerprintStatus::ERROR_COMM;
    }

    if (receivePacket() == 0) {
        return FingerprintStatus::ERROR_TIMEOUT;
    }

    FingerprintStatus status = static_cast<FingerprintStatus>(getConfirmationCode());
    return status;
}

FingerprintStatus R307S_Driver::fingerFastSearch(FingerprintMatch& match) {
    if (!is_initialized_) {
        return FingerprintStatus::ERROR_COMM;
    }

    match.matched = false;
    match.id = 0;
    match.confidence = 0;

    uint8_t data[5];
    data[0] = 0x01;     // Buffer ID (CharBuffer1)
    writeU16(&data[1], 0);      // Start page (0 = first page)
    writeU16(&data[3], 0xFFFF); // Page count (0xFFFF = all pages)
    
    if (!sendPacket(CMD_SEARCH, data, 5)) {
        return FingerprintStatus::ERROR_COMM;
    }

    if (receivePacket(DEFAULT_TIMEOUT_MS * 2) == 0) {
        return FingerprintStatus::ERROR_TIMEOUT;
    }

    FingerprintStatus status = static_cast<FingerprintStatus>(getConfirmationCode());
    
    if (status == FingerprintStatus::OK) {
        // Parse the response: confirmation_code + page_id(2) + match_score(2)
        if (packet_buffer_[6] == PID_ACK) {
            match.matched = true;
            match.id = readU16(&packet_buffer_[10]);
            match.confidence = readU16(&packet_buffer_[12]);
        }
    }

    return status;
}

FingerprintStatus R307S_Driver::deleteModel(uint16_t id) {
    if (!is_initialized_) {
        return FingerprintStatus::ERROR_COMM;
    }

    uint8_t data[4];
    writeU16(&data[0], id);     // Page ID (position)
    writeU16(&data[2], 1);      // Delete count (delete 1 template)
    
    if (!sendPacket(CMD_DELETE, data, 4)) {
        return FingerprintStatus::ERROR_COMM;
    }

    if (receivePacket() == 0) {
        return FingerprintStatus::ERROR_TIMEOUT;
    }

    return static_cast<FingerprintStatus>(getConfirmationCode());
}

FingerprintStatus R307S_Driver::emptyDatabase() {
    if (!is_initialized_) {
        return FingerprintStatus::ERROR_COMM;
    }

    if (!sendPacket(CMD_EMPTY)) {
        return FingerprintStatus::ERROR_COMM;
    }

    if (receivePacket(DEFAULT_TIMEOUT_MS * 2) == 0) {
        return FingerprintStatus::ERROR_TIMEOUT;
    }

    return static_cast<FingerprintStatus>(getConfirmationCode());
}

FingerprintStatus R307S_Driver::getTemplateCount(uint16_t& count) {
    if (!is_initialized_) {
        return FingerprintStatus::ERROR_COMM;
    }

    count = 0;

    if (!sendPacket(CMD_TEMPLATE_NUM)) {
        return FingerprintStatus::ERROR_COMM;
    }

    if (receivePacket() == 0) {
        return FingerprintStatus::ERROR_TIMEOUT;
    }

    FingerprintStatus status = static_cast<FingerprintStatus>(getConfirmationCode());
    
    if (status == FingerprintStatus::OK) {
        // Parse the response: confirmation_code + template_count(2)
        count = readU16(&packet_buffer_[10]);
    }

    return status;
}

FingerprintStatus R307S_Driver::enrollFingerprint(uint16_t id) {
    if (!is_initialized_) {
        return FingerprintStatus::ERROR_COMM;
    }

    // Step 1: Get first image
    FingerprintStatus status = getImage();
    if (status != FingerprintStatus::OK) {
        return status;
    }

    // Convert first image to template in slot 1
    status = image2Tz(1);
    if (status != FingerprintStatus::OK) {
        return status;
    }

    // Small delay between scans
    sleep_ms(500);

    // Step 2: Get second image (user should remove and replace finger)
    status = getImage();
    if (status != FingerprintStatus::OK) {
        return status;
    }

    // Convert second image to template in slot 2
    status = image2Tz(2);
    if (status != FingerprintStatus::OK) {
        return status;
    }

    // Step 3: Create model from both templates
    status = createModel();
    if (status != FingerprintStatus::OK) {
        return status;
    }

    // Step 4: Store the model
    status = storeModel(id);
    return status;
}

FingerprintStatus R307S_Driver::matchFingerprint(FingerprintMatch& match) {
    if (!is_initialized_) {
        return FingerprintStatus::ERROR_COMM;
    }

    // Step 1: Get image
    FingerprintStatus status = getImage();
    if (status != FingerprintStatus::OK) {
        return status;
    }

    // Step 2: Convert image to template
    status = image2Tz(1);
    if (status != FingerprintStatus::OK) {
        return status;
    }

    // Step 3: Search for match
    status = fingerFastSearch(match);
    return status;
}

FingerprintStatus R307S_Driver::loadTemplate(uint16_t id, uint8_t slot) {
    if (!is_initialized_) {
        return FingerprintStatus::ERROR_COMM;
    }

    if (slot != 1 && slot != 2) {
        return FingerprintStatus::ERROR_BAD_LOCATION;
    }

    uint8_t data[3];
    data[0] = slot;  // Buffer ID (CharBuffer 1 or 2)
    writeU16(&data[1], id);  // Page ID (template location)
    
    if (!sendPacket(CMD_LOAD, data, 3)) {
        return FingerprintStatus::ERROR_COMM;
    }

    if (receivePacket() == 0) {
        return FingerprintStatus::ERROR_TIMEOUT;
    }

    return static_cast<FingerprintStatus>(getConfirmationCode());
}

FingerprintStatus R307S_Driver::uploadTemplate(uint8_t slot, const std::vector<uint8_t>& data) {
    if (!is_initialized_) {
        return FingerprintStatus::ERROR_COMM;
    }

    if (slot != 1 && slot != 2) {
        return FingerprintStatus::ERROR_BAD_LOCATION;
    }

    if (data.empty()) {
        return FingerprintStatus::ERROR_UPLOAD_FAIL;
    }

    uint8_t cmd_data[1] = { slot };
    
    // Clear any stale data in RX buffer before starting
    uart_hal_->clearRxBuffer();
    
    if (!sendPacket(CMD_DOWNLOAD_CHAR, cmd_data, 1)) {
        return FingerprintStatus::ERROR_COMM;
    }

    if (receivePacket() == 0) {
        return FingerprintStatus::ERROR_TIMEOUT;
    }

    FingerprintStatus status = static_cast<FingerprintStatus>(getConfirmationCode());
    if (status != FingerprintStatus::OK) {
        return status;
    }

    // Send template data in packets
    printf("[R307S] Sending %zu bytes in data packets (%zu bytes/packet)...\n", data.size(), data_packet_size_);
    if (!sendDataPacket(data)) {
        printf("[R307S] ERROR: sendDataPacket failed\n");
        return FingerprintStatus::ERROR_UPLOAD_FAIL;
    }
    printf("[R307S] All data packets sent\n");

    // R307S does NOT send a confirmation after receiving data packets for DownChar (0x09).
    // The data is loaded into the CharBuffer once the end-data packet is received.
    // Wait a bit for the sensor to process.
    sleep_ms(100);
    
    return FingerprintStatus::OK;
}

FingerprintStatus R307S_Driver::downloadTemplate(uint8_t slot, std::vector<uint8_t>& data) {
    if (!is_initialized_) {
        printf("[R307S] downloadTemplate: Not initialized\n");
        return FingerprintStatus::ERROR_COMM;
    }

    if (slot != 1 && slot != 2) {
        printf("[R307S] downloadTemplate: Invalid slot %d\n", slot);
        return FingerprintStatus::ERROR_BAD_LOCATION;
    }

    data.clear();

    uint8_t cmd_data[1] = { slot };
    
    printf("[R307S] Sending CMD_UPLOAD_CHAR for slot %d\n", slot);
    if (!sendPacket(CMD_UPLOAD_CHAR, cmd_data, 1)) {
        printf("[R307S] downloadTemplate: Failed to send packet\n");
        return FingerprintStatus::ERROR_COMM;
    }

    printf("[R307S] Waiting for ACK...\n");
    if (receivePacket() == 0) {
        printf("[R307S] downloadTemplate: Timeout waiting for ACK\n");
        return FingerprintStatus::ERROR_TIMEOUT;
    }

    FingerprintStatus status = static_cast<FingerprintStatus>(getConfirmationCode());
    printf("[R307S] ACK received, status: 0x%02X\n", static_cast<int>(status));
    if (status != FingerprintStatus::OK) {
        return status;
    }

    // Receive template data packets
    printf("[R307S] Receiving data packets...\n");
    size_t received = receiveDataPacket(data, DEFAULT_TIMEOUT_MS * 3);
    printf("[R307S] Received %zu bytes\n", received);
    
    if (received == 0) {
        printf("[R307S] downloadTemplate: receiveDataPacket failed\n");
        return FingerprintStatus::ERROR_RECV_FAIL;
    }

    printf("[R307S] Download successful, %zu bytes\n", data.size());
    return FingerprintStatus::OK;
}

FingerprintStatus R307S_Driver::compareTemplates(uint16_t& confidence) {
    if (!is_initialized_) {
        return FingerprintStatus::ERROR_COMM;
    }

    confidence = 0;

    if (!sendPacket(CMD_MATCH)) {
        return FingerprintStatus::ERROR_COMM;
    }

    if (receivePacket() == 0) {
        return FingerprintStatus::ERROR_TIMEOUT;
    }

    FingerprintStatus status = static_cast<FingerprintStatus>(getConfirmationCode());
    
    if (status == FingerprintStatus::OK) {
        // Parse the response: confirmation_code + match_score(2)
        confidence = readU16(&packet_buffer_[10]);
    }

    return status;
}

bool R307S_Driver::setLED(bool on) {
    if (!is_initialized_) {
        return false;
    }

    // LED control command format varies by sensor version
    // This implements a basic on/off control
    uint8_t data[4];
    data[0] = 0x01;  // Control code
    data[1] = on ? 0x01 : 0x00;  // LED state
    data[2] = 0x00;  // Speed (not used for simple on/off)
    data[3] = 0x01;  // Color count (not used for simple on/off)
    
    if (!sendPacket(CMD_LED_CONFIG, data, 4)) {
        return false;
    }

    if (receivePacket() == 0) {
        return false;
    }

    return getConfirmationCode() == static_cast<uint8_t>(FingerprintStatus::OK);
}

FingerprintStatus R307S_Driver::readSysPara(uint16_t& status_register, uint16_t& system_id,
                                           uint16_t& library_size, uint16_t& security_level) {
    if (!is_initialized_) {
        return FingerprintStatus::ERROR_COMM;
    }

    if (!sendPacket(CMD_READ_SYS_PARA)) {
        return FingerprintStatus::ERROR_COMM;
    }

    if (receivePacket() == 0) {
        return FingerprintStatus::ERROR_TIMEOUT;
    }

    FingerprintStatus status = static_cast<FingerprintStatus>(getConfirmationCode());
    
    if (status == FingerprintStatus::OK) {
        // Parse system parameters (16 bytes total after confirmation code)
        status_register = readU16(&packet_buffer_[10]);
        system_id = readU16(&packet_buffer_[12]);
        library_size = readU16(&packet_buffer_[14]);
        security_level = readU16(&packet_buffer_[16]);
    }

    return status;
}

// Private Methods - Protocol Implementation

bool R307S_Driver::sendPacket(uint8_t command, const uint8_t* data, size_t data_len) {
    if (uart_hal_ == nullptr) {
        return false;
    }

    // Packet structure:
    // [Start Code: 2 bytes] [Address: 4 bytes] [PID: 1 byte] 
    // [Length: 2 bytes] [Data: n bytes] [Checksum: 2 bytes]
    
    uint8_t packet[PACKET_BUFFER_SIZE];
    size_t idx = 0;

    // Start code (0xEF01)
    writeU16(&packet[idx], START_CODE);
    idx += 2;

    // Address (4 bytes)
    writeU32(&packet[idx], address_);
    idx += 4;

    // Package Identifier (Command packet)
    packet[idx++] = PID_COMMAND;

    // Length = instruction + data length + checksum length
    uint16_t length = 1 + data_len + 2;
    writeU16(&packet[idx], length);
    idx += 2;

    // Instruction/Command
    packet[idx++] = command;

    // Data
    if (data != nullptr && data_len > 0) {
        memcpy(&packet[idx], data, data_len);
        idx += data_len;
    }

    // Calculate checksum (PID + Length + Instruction + Data)
    uint16_t checksum = calculateChecksum(&packet[6], idx - 6);
    writeU16(&packet[idx], checksum);
    idx += 2;

    // Send the packet
    for (size_t i = 0; i < idx; i++) {
        uart_hal_->writeByte(packet[i]);
    }

    return true;
}

size_t R307S_Driver::receivePacket(uint32_t timeout_ms) {
    if (uart_hal_ == nullptr) {
        return 0;
    }

    memset(packet_buffer_, 0, PACKET_BUFFER_SIZE);
    
    uint32_t start_time = to_ms_since_boot(get_absolute_time());
    size_t idx = 0;

    // Wait for start code (0xEF01)
    while (true) {
        if (uart_hal_->available()) {
            uint8_t byte = uart_hal_->readByte();
            
            if (idx == 0 && byte == 0xEF) {
                packet_buffer_[idx++] = byte;
            } else if (idx == 1) {
                if (byte == 0x01) {
                    packet_buffer_[idx++] = byte;
                    break;  // Found start code
                } else {
                    idx = 0;  // Reset if not matched
                }
            }
        }

        // Check timeout
        if (to_ms_since_boot(get_absolute_time()) - start_time >= timeout_ms) {
            return 0;
        }
        
        sleep_us(100);
    }

    // Read address (4 bytes)
    for (int i = 0; i < 4; i++) {
        while (!uart_hal_->available()) {
            if (to_ms_since_boot(get_absolute_time()) - start_time >= timeout_ms) {
                return 0;
            }
            sleep_us(100);
        }
        packet_buffer_[idx++] = uart_hal_->readByte();
    }

    // Read PID (1 byte)
    while (!uart_hal_->available()) {
        if (to_ms_since_boot(get_absolute_time()) - start_time >= timeout_ms) {
            return 0;
        }
        sleep_us(100);
    }
    packet_buffer_[idx++] = uart_hal_->readByte();

    // Read length (2 bytes)
    for (int i = 0; i < 2; i++) {
        while (!uart_hal_->available()) {
            if (to_ms_since_boot(get_absolute_time()) - start_time >= timeout_ms) {
                return 0;
            }
            sleep_us(100);
        }
        packet_buffer_[idx++] = uart_hal_->readByte();
    }

    // Get the length value
    uint16_t packet_len = readU16(&packet_buffer_[7]);
    
    // Read remaining data (length includes checksum)
    for (uint16_t i = 0; i < packet_len; i++) {
        while (!uart_hal_->available()) {
            if (to_ms_since_boot(get_absolute_time()) - start_time >= timeout_ms) {
                return 0;
            }
            sleep_us(100);
        }
        packet_buffer_[idx++] = uart_hal_->readByte();
    }

    // Verify checksum
    uint16_t received_checksum = readU16(&packet_buffer_[idx - 2]);
    uint16_t calculated_checksum = calculateChecksum(&packet_buffer_[6], idx - 8);
    
    if (received_checksum != calculated_checksum) {
        return 0;  // Checksum mismatch
    }

    return idx;
}

size_t R307S_Driver::receiveDataPacket(std::vector<uint8_t>& data, uint32_t timeout_ms) {
    if (uart_hal_ == nullptr) {
        return 0;
    }

    data.clear();
    uint32_t start_time = to_ms_since_boot(get_absolute_time());
    bool end_received = false;

    while (!end_received) {
        uint8_t temp_buffer[PACKET_BUFFER_SIZE];
        memset(temp_buffer, 0, PACKET_BUFFER_SIZE);
        size_t idx = 0;

        // Wait for start code (0xEF01)
        while (true) {
            if (uart_hal_->available()) {
                uint8_t byte = uart_hal_->readByte();
                
                if (idx == 0 && byte == 0xEF) {
                    temp_buffer[idx++] = byte;
                } else if (idx == 1) {
                    if (byte == 0x01) {
                        temp_buffer[idx++] = byte;
                        break;
                    } else {
                        idx = 0;
                    }
                }
            }

            if (to_ms_since_boot(get_absolute_time()) - start_time >= timeout_ms) {
                return 0;
            }
            
            sleep_us(100);
        }

        // Read address (4 bytes)
        for (int i = 0; i < 4; i++) {
            while (!uart_hal_->available()) {
                if (to_ms_since_boot(get_absolute_time()) - start_time >= timeout_ms) {
                    return 0;
                }
                sleep_us(100);
            }
            temp_buffer[idx++] = uart_hal_->readByte();
        }

        // Read PID (1 byte)
        while (!uart_hal_->available()) {
            if (to_ms_since_boot(get_absolute_time()) - start_time >= timeout_ms) {
                return 0;
            }
            sleep_us(100);
        }
        uint8_t pid = uart_hal_->readByte();
        temp_buffer[idx++] = pid;

        // Check if this is a data packet or end data packet
        if (pid == PID_DATA) {
            end_received = false;
        } else if (pid == PID_END_DATA) {
            end_received = true;
        } else {
            return 0;  // Unexpected packet type
        }

        // Read length (2 bytes)
        for (int i = 0; i < 2; i++) {
            while (!uart_hal_->available()) {
                if (to_ms_since_boot(get_absolute_time()) - start_time >= timeout_ms) {
                    return 0;
                }
                sleep_us(100);
            }
            temp_buffer[idx++] = uart_hal_->readByte();
        }

        uint16_t packet_len = readU16(&temp_buffer[7]);
        
        // Read data and checksum (packet_len includes data + checksum)
        for (uint16_t i = 0; i < packet_len; i++) {
            while (!uart_hal_->available()) {
                if (to_ms_since_boot(get_absolute_time()) - start_time >= timeout_ms) {
                    return 0;
                }
                sleep_us(100);
            }
            temp_buffer[idx++] = uart_hal_->readByte();
        }

        // Verify checksum
        uint16_t received_checksum = readU16(&temp_buffer[idx - 2]);
        uint16_t calculated_checksum = calculateChecksum(&temp_buffer[6], idx - 8);
        
        if (received_checksum != calculated_checksum) {
            return 0;  // Checksum mismatch
        }

        // Extract data (skip header and checksum)
        // Data starts at index 9 and ends at idx - 2 (before checksum)
        for (size_t i = 9; i < idx - 2; i++) {
            data.push_back(temp_buffer[i]);
        }
    }

    return data.size();
}

bool R307S_Driver::sendDataPacket(const std::vector<uint8_t>& data) {
    if (uart_hal_ == nullptr || data.empty()) {
        return false;
    }

    // Use sensor's configured packet size (read during init)
    const size_t PACKET_DATA_SIZE = data_packet_size_;
    
    size_t total_sent = 0;
    size_t remaining = data.size();
    int packet_num = 0;

    while (remaining > 0) {
        uint8_t packet[PACKET_BUFFER_SIZE];
        size_t idx = 0;

        // Start code (0xEF01)
        writeU16(&packet[idx], START_CODE);
        idx += 2;

        // Address (4 bytes)
        writeU32(&packet[idx], address_);
        idx += 4;

        // Determine packet type
        size_t chunk_size = (remaining <= PACKET_DATA_SIZE) ? remaining : PACKET_DATA_SIZE;
        uint8_t pid = (remaining <= PACKET_DATA_SIZE) ? PID_END_DATA : PID_DATA;
        packet[idx++] = pid;

        // Length = data + checksum
        uint16_t length = chunk_size + 2;
        writeU16(&packet[idx], length);
        idx += 2;

        // Copy data chunk
        for (size_t i = 0; i < chunk_size; i++) {
            packet[idx++] = data[total_sent + i];
        }

        // Calculate and add checksum
        uint16_t checksum = calculateChecksum(&packet[6], idx - 6);
        writeU16(&packet[idx], checksum);
        idx += 2;

        // Send packet
        for (size_t i = 0; i < idx; i++) {
            uart_hal_->writeByte(packet[i]);
        }
        
        packet_num++;
        printf("[R307S] Sent packet %d: %zu bytes data, PID=0x%02X, total=%zu/%zu\n",
               packet_num, chunk_size, pid, total_sent + chunk_size, data.size());

        total_sent += chunk_size;
        remaining -= chunk_size;

        // Delay between packets to allow sensor to process
        sleep_ms(20);
    }

    return true;
}

uint8_t R307S_Driver::getConfirmationCode() {
    // Confirmation code is at index 9 (after header, address, PID, length)
    return packet_buffer_[9];
}

uint16_t R307S_Driver::calculateChecksum(const uint8_t* buffer, size_t len) {
    uint16_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum += buffer[i];
    }
    return sum;
}

void R307S_Driver::writeU16(uint8_t* buffer, uint16_t value) {
    buffer[0] = (value >> 8) & 0xFF;  // High byte
    buffer[1] = value & 0xFF;          // Low byte
}

void R307S_Driver::writeU32(uint8_t* buffer, uint32_t value) {
    buffer[0] = (value >> 24) & 0xFF;  // Highest byte
    buffer[1] = (value >> 16) & 0xFF;
    buffer[2] = (value >> 8) & 0xFF;
    buffer[3] = value & 0xFF;          // Lowest byte
}

uint16_t R307S_Driver::readU16(const uint8_t* buffer) {
    return (static_cast<uint16_t>(buffer[0]) << 8) | buffer[1];
}

uint32_t R307S_Driver::readU32(const uint8_t* buffer) {
    return (static_cast<uint32_t>(buffer[0]) << 24) |
           (static_cast<uint32_t>(buffer[1]) << 16) |
           (static_cast<uint32_t>(buffer[2]) << 8) |
           static_cast<uint32_t>(buffer[3]);
}