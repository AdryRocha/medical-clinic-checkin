# 1. Find all source and header files
file(GLOB_RECURSE SCREEN_SOURCES 
    CONFIGURE_DEPENDS
    "*.cpp"
    "*.h"
    "*.hpp"
)

# 2. Create the library
add_library(ui_screens ${SCREEN_SOURCES})

# 3. Include paths needed by the UI screens
target_include_directories(ui_screens PUBLIC
    .
    ${CMAKE_SOURCE_DIR}/src                # <--- agora encontra core, services, tasks, hal
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/services
    ${CMAKE_SOURCE_DIR}/src/tasks
    ${CMAKE_SOURCE_DIR}/src/hal
    ${CMAKE_SOURCE_DIR}/src/hal/adapters
    ${CMAKE_SOURCE_DIR}/src/drivers
)

# 4. Link dependencies
target_link_libraries(ui_screens
    pico_stdlib
    lvgl
    FreeRTOS-Kernel
)
