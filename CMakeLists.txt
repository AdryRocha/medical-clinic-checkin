cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Set any variables required for importing libraries
if (DEFINED ENV{FREERTOS_PATH})
  SET(FREERTOS_PATH $ENV{FREERTOS_PATH})
else()
  SET(FREERTOS_PATH ${CMAKE_CURRENT_LIST_DIR}/lib/FreeRTOS-Kernel)
endif()

# Set path for no-OS-FatFS library
SET(NO_OS_FATFS_PATH ${CMAKE_CURRENT_LIST_DIR}/lib/no-OS-FatFS)

include(pico_sdk_import.cmake)
include(${FREERTOS_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)


project(medical-clinic-checkin-pico C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

include(FetchContent)

set(LV_CONF_PATH ${CMAKE_CURRENT_LIST_DIR}/config/lv_conf.h CACHE STRING "" FORCE)

FetchContent_Declare(
        lvgl
        GIT_REPOSITORY https://github.com/lvgl/lvgl.git
        GIT_TAG v8.3.11
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(lvgl)

# Link LVGL with pico libraries to get all necessary Pico SDK includes and dependencies
target_link_libraries(lvgl PUBLIC 
    pico_base
    hardware_sync
    hardware_irq
)

target_include_directories(lvgl PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/config
    ${FREERTOS_PATH}/include
    ${FREERTOS_PATH}/portable/ThirdParty/GCC/RP2040/include
)

# Add no-OS-FatFS library
add_subdirectory(${NO_OS_FATFS_PATH}/FatFs_SPI build_fatfs)

# Add cJSON library for JSON parsing
# Disable tests and examples to avoid linker issues with bare metal target
set(ENABLE_CJSON_TEST OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ENABLE_CJSON_UTILS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
        cjson
        GIT_REPOSITORY https://github.com/DaveGamble/cJSON.git
        GIT_TAG v1.7.18
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(cjson)

# Disable stack protector for cJSON to avoid linker errors
if(TARGET cjson)
    target_compile_options(cjson PRIVATE -fno-stack-protector)
endif()

# Add Mongoose networking library
FetchContent_Declare(
        mongoose
        GIT_REPOSITORY https://github.com/cesanta/mongoose.git
        GIT_TAG 7.20
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(mongoose)

# Create a mongoose library target
add_library(mongoose_lib STATIC
    ${mongoose_SOURCE_DIR}/mongoose.c
)

target_include_directories(mongoose_lib PUBLIC
    ${mongoose_SOURCE_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/config
    ${FREERTOS_PATH}/include
    ${FREERTOS_PATH}/portable/ThirdParty/GCC/RP2040/include
)

target_link_libraries(mongoose_lib PUBLIC
    pico_stdlib
    pico_cyw43_arch_lwip_sys_freertos
    FreeRTOS-Kernel
)

set(MAIN_TARGET_NAME ${PROJECT_NAME})

# Option to select which build to compile
set(BUILD_TARGET "MAIN" CACHE STRING "Choose build target: MAIN, LVGL_DEMO, QR_DEMO, FINGERPRINT_DEMO, SDCARD_DEMO, or WIFI_DEMO")
set_property(CACHE BUILD_TARGET PROPERTY STRINGS "MAIN" "LVGL_DEMO" "QR_DEMO" "FINGERPRINT_DEMO" "SDCARD_DEMO" "WIFI_DEMO")

# Recursively collect all source files from src/ directory and its subdirectories
file(GLOB_RECURSE SRC_FILES
        "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/*.c"
)

# Add hw_config.c from config folder (required by FatFS library)
list(APPEND SRC_FILES "${CMAKE_CURRENT_LIST_DIR}/config/hw_config.c")

# Remove files based on BUILD_TARGET option
if(BUILD_TARGET STREQUAL "LVGL_DEMO")
    message(STATUS "Building LVGL RTOS demo")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/main\\.cpp$")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/tests/code_examples/qr_code_reader_demo\\.cpp$")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/tests/code_examples/fingerprint_demo\\.cpp$")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/tests/code_examples/sdcard_fatfs_demo\\.cpp$")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/tests/code_examples/wifi_demo\\.cpp$")
elseif(BUILD_TARGET STREQUAL "QR_DEMO")
    message(STATUS "Building QR Reader demo")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/main\\.cpp$")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/tests/code_examples/lvgl_rtos_demo\\.cpp$")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/tests/code_examples/fingerprint_demo\\.cpp$")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/tests/code_examples/sdcard_fatfs_demo\\.cpp$")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/tests/code_examples/wifi_demo\\.cpp$")
elseif(BUILD_TARGET STREQUAL "FINGERPRINT_DEMO")
    message(STATUS "Building Fingerprint demo")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/main\\.cpp$")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/tests/code_examples/lvgl_rtos_demo\\.cpp$")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/tests/code_examples/qr_code_reader_demo\\.cpp$")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/tests/code_examples/sdcard_fatfs_demo\\.cpp$")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/tests/code_examples/wifi_demo\\.cpp$")
elseif(BUILD_TARGET STREQUAL "SDCARD_DEMO")
    message(STATUS "Building SD Card FatFS demo")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/main\\.cpp$")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/tests/code_examples/lvgl_rtos_demo\\.cpp$")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/tests/code_examples/qr_code_reader_demo\\.cpp$")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/tests/code_examples/fingerprint_demo\\.cpp$")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/tests/code_examples/sdcard_demo\\.cpp$")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/tests/code_examples/wifi_demo\\.cpp$")
elseif(BUILD_TARGET STREQUAL "WIFI_DEMO")
    message(STATUS "Building WiFi demo")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/main\\.cpp$")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/tests/code_examples/lvgl_rtos_demo\\.cpp$")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/tests/code_examples/qr_code_reader_demo\\.cpp$")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/tests/code_examples/fingerprint_demo\\.cpp$")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/tests/code_examples/sdcard_fatfs_demo\\.cpp$")
else()
    message(STATUS "Building main application")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/tests/code_examples/lvgl_rtos_demo\\.cpp$")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/tests/code_examples/qr_code_reader_demo\\.cpp$")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/tests/code_examples/fingerprint_demo\\.cpp$")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/tests/code_examples/sdcard_fatfs_demo\\.cpp$")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/tests/code_examples/wifi_demo\\.cpp$")
endif()

# Add executable. Default name is the project name, version 0.1
add_executable(${MAIN_TARGET_NAME}
        ${SRC_FILES}
)

pico_set_program_name(${MAIN_TARGET_NAME} "${MAIN_TARGET_NAME}")
pico_set_program_version(${MAIN_TARGET_NAME} "0.0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(${MAIN_TARGET_NAME} 0)
pico_enable_stdio_usb(${MAIN_TARGET_NAME} 1)

# Add the standard library to the build
target_link_libraries(${MAIN_TARGET_NAME}
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(${MAIN_TARGET_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/config
        ${CMAKE_CURRENT_LIST_DIR}/lib
        ${CMAKE_CURRENT_LIST_DIR}/src
        ${NO_OS_FATFS_PATH}/FatFs_SPI/include
        ${cjson_SOURCE_DIR}
        ${mongoose_SOURCE_DIR}
)

# Add any user requested libraries
target_link_libraries(${MAIN_TARGET_NAME} 
        pico_stdlib
        hardware_i2c
        hardware_spi
        hardware_uart
        hardware_rtc
        FreeRTOS-Kernel-Heap4
        lvgl
        cjson
        FatFs_SPI
        pico_cyw43_arch_lwip_sys_freertos
        pico_lwip_http
        pico_lwip_sntp
        mongoose_lib
        )

# Add LVGL demos (only for demo builds, not production)
if(BUILD_TARGET STREQUAL "LVGL_DEMO")
    target_link_libraries(${MAIN_TARGET_NAME} lvgl::demos)
endif()

target_compile_definitions(${MAIN_TARGET_NAME} PRIVATE)

pico_add_extra_outputs(${MAIN_TARGET_NAME})
