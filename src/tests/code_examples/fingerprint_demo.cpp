#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>
#include <string.h>
#include <cstdlib>

#include "pico/stdlib.h"
#include "drivers/fingerprint/r307s/r307s_driver.hpp"
#include "hal/rp2040/hal_uart_rp2040.hpp"
#include "config/pin_config.hpp"
#include "config/fingerprint_config.hpp"

#define LED_PIN             STATUS_PIN_LED

static HAL_UART_RP2040* uart_hal = nullptr;
static R307S_Driver* fp_sensor = nullptr;

// Helper function to convert status code to readable message
const char* getStatusMessage(FingerprintStatus status) {
    switch (status) {
        case FingerprintStatus::OK:
            return "Success";
        case FingerprintStatus::ERROR_COMM:
            return "Communication error";
        case FingerprintStatus::ERROR_NO_FINGER:
            return "No finger detected";
        case FingerprintStatus::ERROR_ENROLL_FAIL:
            return "Enrollment failed";
        case FingerprintStatus::ERROR_BAD_IMAGE:
            return "Failed to capture clear image";
        case FingerprintStatus::ERROR_TOO_MESSY:
            return "Image too messy (clean finger and try again)";
        case FingerprintStatus::ERROR_FEATURE_FAIL:
            return "Failed to extract features (press harder and try again)";
        case FingerprintStatus::ERROR_NO_MATCH:
            return "Finger not recognized";
        case FingerprintStatus::ERROR_NOT_FOUND:
            return "ID not found in database";
        case FingerprintStatus::ERROR_MERGE_FAIL:
            return "Scans too different (use same finger position both times)";
        case FingerprintStatus::ERROR_DELETE_FAIL:
            return "Failed to delete template";
        case FingerprintStatus::ERROR_CLEAR_FAIL:
            return "Failed to clear database";
        case FingerprintStatus::ERROR_WRONG_PASSWORD:
            return "Wrong password";
        case FingerprintStatus::ERROR_NO_TEMPLATE:
            return "No template at this location";
        case FingerprintStatus::ERROR_TIMEOUT:
            return "Operation timed out";
        default:
            return "Unknown error";
    }
}

// Helper function to print menu
void print_menu() {
    printf("\n====== Fingerprint Menu ====\n");
    printf("1. Enroll new fingerprint\n");
    printf("2. Verify fingerprint\n");
    printf("3. Search fingerprint\n");
    printf("4. Delete fingerprint by ID\n");
    printf("5. Clear all fingerprints\n");
    printf("6. Get template count\n");
    printf("7. Read system parameters\n");
    printf("0. Show menu\n");
    printf("==============================\n");
    printf("Enter option: ");
}

// Helper function to read a number from user input
int read_number() {
    char buffer[16];
    int idx = 0;
    
    while (idx < 15) {
        int c = getchar_timeout_us(5000000); // 5 second timeout
        if (c == PICO_ERROR_TIMEOUT) {
            printf("\nTimeout!\n");
            return -1;
        }
        if (c == '\n' || c == '\r') {
            buffer[idx] = '\0';
            printf("\n");
            break;
        }
        if (c >= '0' && c <= '9') {
            buffer[idx++] = c;
            putchar(c);
        }
    }
    
    if (idx == 0) return -1;
    return atoi(buffer);
}

// Enroll a new fingerprint
void enroll_fingerprint() {
    printf("\n--- Enroll New Fingerprint ---\n");
    printf("Enter ID (1-1000): ");
    
    int id = read_number();
    if (id < 1 || id > 1000) {
        printf("Invalid ID!\n");
        return;
    }
    
    printf("\nEnrolling ID %d...\n", id);
    printf("\nSTEP 1: Place your finger on the sensor\n");
    
    // Wait for first image
    FingerprintStatus status = FingerprintStatus::ERROR_NO_FINGER;
    for (int i = 0; i < 50 && status == FingerprintStatus::ERROR_NO_FINGER; i++) {
        status = fp_sensor->getImage();
        if (status == FingerprintStatus::ERROR_NO_FINGER) {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
    
    if (status != FingerprintStatus::OK) {
        printf("Failed to capture first image (status: %d)\n", static_cast<int>(status));
        return;
    }
    printf("First image captured\n");
    
    // Convert to template
    status = fp_sensor->image2Tz(1);
    if (status != FingerprintStatus::OK) {
        printf("Failed to process first image (status: %d)\n", static_cast<int>(status));
        return;
    }
    printf("First template created\n");
    
    // Wait for finger to be removed
    printf("\nSTEP 2: Remove your finger\n");
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Wait until no finger is detected
    int removal_attempts = 0;
    while (removal_attempts < 20) {
        status = fp_sensor->getImage();
        if (status == FingerprintStatus::ERROR_NO_FINGER) {
            printf("Finger removed\n");
            break;
        }
        removal_attempts++;
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    
    // Give extra time before asking for second scan
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    printf("\nSTEP 3: Place the same finger again\n");
    
    // Wait for second image
    status = FingerprintStatus::ERROR_NO_FINGER;
    for (int i = 0; i < 50 && status == FingerprintStatus::ERROR_NO_FINGER; i++) {
        status = fp_sensor->getImage();
        if (status == FingerprintStatus::ERROR_NO_FINGER) {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
    
    if (status != FingerprintStatus::OK) {
        printf("Failed to capture second image (status: %d)\n", static_cast<int>(status));
        return;
    }
    printf("Second image captured\n");
    
    // Convert to template
    status = fp_sensor->image2Tz(2);
    if (status != FingerprintStatus::OK) {
        printf("Failed to process second image (status: %d)\n", static_cast<int>(status));
        return;
    }
    printf("Second template created\n");
    
    // Create model
    status = fp_sensor->createModel();
    if (status != FingerprintStatus::OK) {
        printf("Failed to create model\n");
        printf("  Status: 0x%02X (%d)\n", static_cast<int>(status), static_cast<int>(status));
        
        if (status == FingerprintStatus::ERROR_NOT_FOUND) {
            printf("  Scans too different - try using same finger position\n");
        } else {
            printf("  %s\n", getStatusMessage(status));
        }
        return;
    }
    printf("Model created\n");
    
    // Store model
    status = fp_sensor->storeModel(id);
    if (status != FingerprintStatus::OK) {
        printf("Failed to store fingerprint: %s\n", getStatusMessage(status));
        return;
    }
    
    printf("\nFingerprint enrolled as ID %d\n", id);
}

// Verify a specific fingerprint ID
void verify_fingerprint() {
    printf("\n--- Verify Fingerprint ---\n");
    printf("Enter ID to verify (1-1000): ");
    
    int id = read_number();
    if (id < 1 || id > 1000) {
        printf("Invalid ID!\n");
        return;
    }
    
    printf("\nPlace your finger on the sensor\n");
    
    // Wait for finger to be placed
    FingerprintStatus status;
    int attempts = 0;
    const int MAX_ATTEMPTS = 10;
    
    while (attempts < MAX_ATTEMPTS) {
        status = fp_sensor->getImage();
        if (status == FingerprintStatus::OK) {
            printf("Finger detected\n");
            break;
        }
        attempts++;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    
    if (attempts >= MAX_ATTEMPTS) {
        printf("Timeout - no finger detected\n");
        return;
    }
    
    // Convert image to template in CharBuffer1
    status = fp_sensor->image2Tz(1);
    if (status != FingerprintStatus::OK) {
        printf("Failed to process fingerprint: %s\n", getStatusMessage(status));
        return;
    }
    printf("Fingerprint processed\n");
    
    // Load the stored template for this ID into CharBuffer2
    status = fp_sensor->loadTemplate(id, 2);
    if (status != FingerprintStatus::OK) {
        printf("Failed to load template for ID %d: %s\n", id, getStatusMessage(status));
        printf("(ID may not be enrolled)\n");
        return;
    }
    printf("Template loaded for ID %d\n", id);
    
    // Compare CharBuffer1 (current finger) with CharBuffer2 (stored template)
    uint16_t confidence = 0;
    status = fp_sensor->compareTemplates(confidence);
    
    if (status == FingerprintStatus::OK) {
        printf("Verified - ID %d matches (confidence: %d)\n", id, confidence);
    } else {
        printf("Not verified - fingerprint does not match ID %d\n", id);
        printf("%s\n", getStatusMessage(status));
    }
}

// Search for a fingerprint in the database
void search_fingerprint() {
    printf("\n--- Search Fingerprint ---\n");
    printf("\nPlace your finger on the sensor\n");
    
    // Wait for finger to be placed
    FingerprintStatus status;
    int attempts = 0;
    const int MAX_ATTEMPTS = 10;
    
    while (attempts < MAX_ATTEMPTS) {
        status = fp_sensor->getImage();
        if (status == FingerprintStatus::OK) {
            printf("Finger detected\n");
            break;
        }
        attempts++;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    
    if (attempts >= MAX_ATTEMPTS) {
        printf("Timeout - no finger detected\n");
        return;
    }
    
    // Convert image to template
    status = fp_sensor->image2Tz(1);
    if (status != FingerprintStatus::OK) {
        printf("Failed to process fingerprint: %s\n", getStatusMessage(status));
        return;
    }
    printf("Fingerprint processed\n");
    
    // Search for match
    FingerprintMatch match;
    status = fp_sensor->fingerFastSearch(match);
    
    if (status == FingerprintStatus::OK && match.matched) {
        printf("\nMatch found: ID %d (confidence: %d)\n", match.id, match.confidence);
    } else if (status == FingerprintStatus::ERROR_NO_MATCH) {
        printf("\nNo match - finger not in database\n");
    } else if (status == FingerprintStatus::OK) {
        printf("\nNo match found\n");
    } else {
        printf("\nSearch failed: %s\n", getStatusMessage(status));
    }
}

// Delete a fingerprint by ID
void delete_fingerprint() {
    printf("\n--- Delete Fingerprint ---\n");
    printf("Enter ID to delete (1-1000): ");
    
    int id = read_number();
    if (id < 1 || id > 1000) {
        printf("Invalid ID!\n");
        return;
    }
    
    FingerprintStatus status = fp_sensor->deleteModel(id);
    if (status == FingerprintStatus::OK) {
        printf("Fingerprint ID %d deleted\n", id);
    } else {
        printf("Failed to delete ID %d: %s\n", id, getStatusMessage(status));
    }
}

// Clear all fingerprints
void clear_all_fingerprints() {
    printf("\n--- Clear All Fingerprints ---\n");
    printf("Are you sure? This will delete ALL fingerprints!\n");
    printf("Press 'Y' to confirm: ");
    
    int c = getchar_timeout_us(5000000);
    printf("\n");
    
    if (c == 'Y' || c == 'y') {
        FingerprintStatus status = fp_sensor->emptyDatabase();
        if (status == FingerprintStatus::OK) {
            printf("All fingerprints cleared\n");
        } else {
            printf("Failed to clear database (status: %d)\n", static_cast<int>(status));
        }
    } else {
        printf("Cancelled\n");
    }
}

// Get template count
void get_template_count() {
    printf("\n--- Template Count ---\n");
    
    uint16_t count = 0;
    FingerprintStatus status = fp_sensor->getTemplateCount(count);
    if (status == FingerprintStatus::OK) {
        printf("Stored fingerprints: %d\n", count);
    } else {
        printf("Failed to get template count (status: %d)\n", static_cast<int>(status));
    }
}

// Read system parameters
void read_system_params() {
    printf("\n--- System Parameters ---\n");
    
    uint16_t status_register = 0;
    uint16_t system_id = 0;
    uint16_t library_size = 0;
    uint16_t security_level = 0;
    
    FingerprintStatus status = fp_sensor->readSysPara(status_register, system_id, library_size, security_level);
    if (status == FingerprintStatus::OK) {
        printf("System parameters:\n");
        printf("  Status: 0x%04X\n", status_register);
        printf("  System ID: 0x%04X\n", system_id);
        printf("  Library Size: %d\n", library_size);
        printf("  Security Level: %d\n", security_level);
    } else {
        printf("Failed to read parameters (status: %d)\n", static_cast<int>(status));
    }
}

// Main fingerprint task
void fingerprint_task(void *pvParameters) {
    printf("Initializing R307S fingerprint sensor...\n");
    
    // Initialize UART
    uart_hal = new HAL_UART_RP2040(FP_UART_INSTANCE, FINGERPRINT_PIN_TX, FINGERPRINT_PIN_RX);
    if (!uart_hal->init(FP_BAUDRATE, FP_DATA_BITS, FP_STOP_BITS, FP_PARITY)) {
        printf("ERROR: UART init failed!\n");
        vTaskDelete(NULL);
        return;
    }
    
    // Initialize fingerprint sensor
    fp_sensor = new R307S_Driver(uart_hal);
    if (!fp_sensor->init()) {
        printf("ERROR: Sensor init failed - check wiring and power\n");
        vTaskDelete(NULL);
        return;
    }
    
    printf("Sensor ready\n");
    
    // Get initial template count
    uint16_t count = 0;
    FingerprintStatus status = fp_sensor->getTemplateCount(count);
    if (status == FingerprintStatus::OK) {
        printf("Current stored fingerprints: %d\n", count);
    }
    
    print_menu();
    
    // Main loop
    while (true) {
        int c = getchar_timeout_us(100000); // 100ms timeout
        
        if (c != PICO_ERROR_TIMEOUT) {
            switch (c) {
                case '0':
                    print_menu();
                    break;
                    
                case '1':
                    enroll_fingerprint();
                    break;
                    
                case '2':
                    verify_fingerprint();
                    break;
                    
                case '3':
                    search_fingerprint();
                    break;
                    
                case '4':
                    delete_fingerprint();
                    break;
                    
                case '5':
                    clear_all_fingerprints();
                    break;
                    
                case '6':
                    get_template_count();
                    break;
                    
                case '7':
                    read_system_params();
                    break;
                    
                default:
                    printf("\nInvalid option! Press 0 for menu.\n");
                    break;
            }
            
            printf("\nReady for next command (press 0 for menu): ");
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// Status LED blink task
void blink_task(void *pvParameters) {
    while (true) {
        gpio_put(LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
        gpio_put(LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

int main() {
    // Initialize stdio
    stdio_init_all();
    sleep_ms(2000);
    
    printf("\n");
    printf("=============================\n");
    printf("  R307S Fingerprint Demo\n");
    printf("=============================\n");
    printf("UART%d @ %d baud (TX=%d, RX=%d)\n\n", 
           FP_UART_INSTANCE == uart0 ? 0 : 1, 
           FP_BAUDRATE, FINGERPRINT_PIN_TX, FINGERPRINT_PIN_RX);
    
    // Initialize LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    
    // Create tasks
    xTaskCreate(fingerprint_task, "Fingerprint", 2048, NULL, 2, NULL);
    xTaskCreate(blink_task, "Blink", 128, NULL, 1, NULL);
    
    // Start scheduler
    vTaskStartScheduler();
    
    // Should never reach here
    while (1) { }
    return 0;
}
