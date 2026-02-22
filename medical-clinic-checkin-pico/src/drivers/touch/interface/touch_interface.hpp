#pragma once
#include <cstdint>

// Definição da estrutura do Ponto de Toque
// Se você já tem isso definido em outro lugar, pode remover e incluir o header certo.
#ifndef TOUCH_POINT_STRUCT
#define TOUCH_POINT_STRUCT
struct TouchPoint {
    uint16_t x;
    uint16_t y;
    bool valid;
};
#endif

class TouchInterface {
public:
    virtual ~TouchInterface() = default;
    virtual bool init() = 0;
    virtual bool isTouched() = 0;
    virtual bool readPoint(TouchPoint* point) = 0;
    virtual uint8_t getMaxTouchPoints() const { return 1; }
};