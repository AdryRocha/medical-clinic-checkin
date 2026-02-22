#ifndef BOARD_FACTORY_HPP
#define BOARD_FACTORY_HPP

#include <memory>
#include "board_context.hpp"

class BoardFactory {
public:
    static BoardContext create_pico2w();
};

#endif // BOARD_FACTORY_HPP
