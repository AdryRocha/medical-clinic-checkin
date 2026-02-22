#pragma once
#include <cstdint>
#include <cstddef>

class HAL_SPI_Interface {
public:
    virtual ~HAL_SPI_Interface() = default;

    virtual bool init(uint32_t baudrate) = 0;
    virtual bool transfer(const uint8_t *tx, uint8_t *rx, size_t len) = 0;
    virtual bool write(const uint8_t *data, size_t len) = 0;
};
