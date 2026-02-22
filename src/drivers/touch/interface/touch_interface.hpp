#ifndef TOUCH_INTERFACE_HPP
#define TOUCH_INTERFACE_HPP

#include <stdint.h>

/**
 * @brief Touch point data structure
 */
struct TouchPoint {
    uint16_t x;         // X coordinate
    uint16_t y;         // Y coordinate
    uint8_t event;      // Touch event type (0=down, 1=up, 2=contact)
    bool valid;         // Whether this touch point is valid
};

/**
 * @brief Abstract interface for touch controllers
 */
class TouchInterface {
public:
    virtual ~TouchInterface() = default;

    /**
     * @brief Initialize the touch controller
     * @return true if initialization was successful
     */
    virtual bool init() = 0;

    /**
     * @brief Read current touch data
     * @param point Pointer to TouchPoint structure to fill
     * @return true if touch data was read successfully
     */
    virtual bool readTouch(TouchPoint* point) = 0;

    /**
     * @brief Check if the screen is currently being touched
     * @return true if touched
     */
    virtual bool isTouched() = 0;

    /**
     * @brief Get the number of simultaneous touches supported
     * @return Maximum number of touch points
     */
    virtual uint8_t getMaxTouchPoints() const = 0;
};

#endif // TOUCH_INTERFACE_HPP