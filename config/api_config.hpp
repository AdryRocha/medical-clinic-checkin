#ifndef API_CONFIG_HPP
#define API_CONFIG_HPP

/**
 * @brief API Configuration
 * Configure the REST API endpoint for appointment synchronization
 */

// API Base URL - Update with your server's IP address or hostname
#define API_BASE_URL "http://192.168.0.171:8000"

// API Endpoints
#define API_ENDPOINT_AUTH "/auth/token"
#define API_ENDPOINT_APPOINTMENTS "/consultas"

// Device Credentials
#define API_DEVICE_USERNAME "admin"
#define API_DEVICE_PASSWORD "senha"

// Timeout configuration (milliseconds)
#define API_DEFAULT_TIMEOUT_MS 30000
#define API_AUTH_TIMEOUT_MS 10000

// Token refresh
#define API_TOKEN_REFRESH_BEFORE_EXPIRE_HOURS 1

// Retry configuration
#define API_MAX_RETRIES 3
#define API_RETRY_DELAY_MS 2000

#endif // API_CONFIG_HPP