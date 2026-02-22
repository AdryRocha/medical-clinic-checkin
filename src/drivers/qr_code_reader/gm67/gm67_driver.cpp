#include "gm67_driver.hpp"
#include "pico/stdlib.h"
#include <cstring>

GM67_Driver::GM67_Driver(HAL_UART_Interface* uart_hal)
    : uart_hal_(uart_hal),
      scan_callback_(nullptr),
      rx_index_(0),
      is_initialized_(false),
      last_scan_time_(0) {
    memset(rx_buffer_, 0, BUFFER_SIZE);
}

bool GM67_Driver::init() {
    if (uart_hal_ == nullptr) {
        return false;
    }

    uart_hal_->clearRxBuffer();
    
    is_initialized_ = true;
    rx_index_ = 0;
    last_scan_time_ = 0;
    
    sleep_ms(100);
    
    return true;
}

size_t GM67_Driver::readScan(char* buffer, size_t max_length, uint32_t timeout_ms) {
    if (!is_initialized_ || buffer == nullptr || max_length == 0) {
        return 0;
    }

    size_t total_read = 0;
    uint32_t start_time = to_ms_since_boot(get_absolute_time());
    uint32_t last_data_time = start_time;
    const uint32_t no_data_timeout_ms = 50;

    while (total_read < max_length - 1) {
        if (uart_hal_->available() > 0) {
            uint8_t temp_buf[256];
            size_t len = uart_hal_->readAvailable(temp_buf, sizeof(temp_buf));
            for (size_t i = 0; i < len && total_read < max_length - 1; i++) {
                char ch = (char)temp_buf[i];
                if (ch == '\r' || ch == '\n') {
                    if (total_read > 0) {
                        buffer[total_read] = '\0';
                        return total_read;
                    }
                    continue;
                }
                buffer[total_read++] = ch;
            }
            last_data_time = to_ms_since_boot(get_absolute_time());
        } else {
            uint32_t elapsed_since_last = to_ms_since_boot(get_absolute_time()) - last_data_time;
            if (elapsed_since_last >= no_data_timeout_ms && total_read > 0) {
                break;
            }
        }

        if (timeout_ms > 0) {
            uint32_t elapsed = to_ms_since_boot(get_absolute_time()) - start_time;
            if (elapsed >= timeout_ms) {
                break;
            }
        }

        sleep_us(100);
    }

    buffer[total_read] = '\0';
    return total_read;
}

bool GM67_Driver::isScanAvailable() {
    return is_initialized_ && uart_hal_->available() > 0;
}

void GM67_Driver::setScanCallback(ScanCallback callback) {
    scan_callback_ = callback;
}

void GM67_Driver::process() {
    if (!is_initialized_) {
        return;
    }

    uint8_t temp_buf[512];
    size_t len;
    do {
        len = uart_hal_->readAvailable(temp_buf, sizeof(temp_buf));
        for (size_t i = 0; i < len; i++) {
            processChar((char)temp_buf[i]);
        }
    } while (len > 0);
}

void GM67_Driver::processChar(char ch) {
    if (ch == '\r' || ch == '\n') {
        if (rx_index_ > 0) {
            rx_buffer_[rx_index_] = '\0';
            handleScanComplete();
            rx_index_ = 0;
        }
        return;
    }

    if (rx_index_ < BUFFER_SIZE - 1) {
        rx_buffer_[rx_index_++] = ch;
    } else {
        printf("GM67 buffer overflow, resetting\n");
        rx_index_ = 0;
    }
}

void GM67_Driver::handleScanComplete() {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    
    if (now - last_scan_time_ < 500) {
        return;
    }
    
    last_scan_time_ = now;
    
    if (scan_callback_ && rx_index_ > 0) {
        std::string scan_data(rx_buffer_, rx_index_);
        scan_callback_(scan_data);
    }
}

bool GM67_Driver::sendCommand(const char* command) {
    if (!is_initialized_ || command == nullptr) {
        return false;
    }

    size_t cmd_len = strlen(command);
    size_t bytes_written = uart_hal_->write(
        reinterpret_cast<const uint8_t*>(command), 
        cmd_len
    );
    
    uart_hal_->flush();
    
    return bytes_written == cmd_len;
}

bool GM67_Driver::enableScan(bool enable) {
    if (enable) {
        return sendCommand("AT+SCAN\r\n");
    } else {
        return sendCommand("AT+STOP\r\n");
    }
}

bool GM67_Driver::triggerScan() {
    return sendCommand("AT+TRIG\r\n");
}

bool GM67_Driver::setContinuousMode() {
    return sendCommand("AT+MODE=1\r\n");
}

bool GM67_Driver::setCommandMode() {
    return sendCommand("AT+MODE=0\r\n");
}