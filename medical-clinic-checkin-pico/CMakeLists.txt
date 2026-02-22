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

# =============================
# BOARD SELECTION
# =============================
set(BOARD "pico2_w" CACHE STRING "Target board")

message(STATUS "Building for BOARD=${BOARD}")

option(HAS_WIFI  "Enable WiFi"  ON)
option(HAS_SD    "Enable SD"    ON)
option(HAS_TOUCH "Enable Touch" ON)

# Pico SDK
set(PICO_BOARD pico2_w)

# Set any variables required for importing libraries
if (DEFINED ENV{FREERTOS_PATH})
  SET(FREERTOS_PATH $ENV{FREERTOS_PATH})
else()
  SET(FREERTOS_PATH ${CMAKE_CURRENT_LIST_DIR}/lib/FreeRTOS-Kernel)
endif()

# Set path for no-OS-FatFS library
SET(NO_OS_FATFS_PATH ${CMAKE_CURRENT_LIST_DIR}/lib/no-OS-FatFS)

# ============================================================
# FreeRTOS Kernel Path (OBRIGATÓRIO para FreeRTOS import)
# ============================================================
set(FREERTOS_KERNEL_PATH
    "${CMAKE_CURRENT_LIST_DIR}/lib/FreeRTOS-Kernel"
    CACHE PATH "Path to FreeRTOS-Kernel"
    FORCE
)

include(pico_sdk_import.cmake)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Condicionalmente inclui o import do FreeRTOS apenas para a plataforma correta
set(FREERTOS_IMPORT_INCLUDED FALSE)

# Nota: PICO_PLATFORM é definido após pico_sdk_init()
if(DEFINED PICO_PLATFORM AND PICO_PLATFORM MATCHES "rp2040")
    if(EXISTS "${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake")
        include("${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake")
        set(FREERTOS_IMPORT_INCLUDED TRUE)
    endif()
elseif(DEFINED PICO_PLATFORM AND PICO_PLATFORM MATCHES "rp2350")
    # RP2350: tenta importar o port oficial; se ausente, segue sem import para usar fallback.
    set(_rp2350_import "${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2350_ARM_NTZ/FreeRTOS_Kernel_import.cmake")
    if(EXISTS "${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2350_ARM_NTZ/non_secure/port.c" AND EXISTS "${_rp2350_import}")
        include("${_rp2350_import}")
        set(FREERTOS_IMPORT_INCLUDED TRUE)
    else()
        message(WARNING "Port RP2350_ARM_NTZ não encontrado; prosseguindo com fallback ARM_CM33_NTZ (menos estável). Atualize o FreeRTOS-Kernel para obter o port RP2350.")
        set(FREERTOS_IMPORT_INCLUDED FALSE)
    endif()
else()
    # Fallback legacy
    if(EXISTS "${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake")
         if(DEFINED PICO_PLATFORM AND PICO_PLATFORM MATCHES "rp2040")
            include("${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake")
            set(FREERTOS_IMPORT_INCLUDED TRUE)
        endif()
    endif()
endif()


project(medical-clinic-checkin-pico C CXX ASM)

# Global Include para Configs
include_directories("${CMAKE_CURRENT_LIST_DIR}/src/config")
include_directories("${CMAKE_CURRENT_LIST_DIR}/config") 

include_directories(${CMAKE_CURRENT_LIST_DIR}/src/config)

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

# Link LVGL with pico libraries
target_link_libraries(lvgl PUBLIC 
    pico_base
    hardware_sync
    hardware_irq
)

target_include_directories(lvgl PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/config
    ${FREERTOS_PATH}/include
)

# Add no-OS-FatFS library
add_subdirectory(${NO_OS_FATFS_PATH}/FatFs_SPI build_fatfs)

set(MAIN_TARGET_NAME ${PROJECT_NAME})

# Option to select which build to compile
set(BUILD_TARGET "MAIN" CACHE STRING "...")
set_property(CACHE BUILD_TARGET PROPERTY STRINGS "MAIN" "LVGL_DEMO" "QR_DEMO" "FINGERPRINT_DEMO" "SDCARD_DEMO" "WIFI_DEMO")

# Recursively collect all source files from src/ directory and its subdirectories
file(GLOB_RECURSE SRC_FILES
        "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/*.c"
)

# ==============================================================================
# [CORREÇÃO CRÍTICA 1] FILTRO ANTI-RP2040
# Removemos qualquer arquivo que tenha "rp2040" no caminho para evitar conflito
# ==============================================================================
list(FILTER SRC_FILES EXCLUDE REGEX ".*/rp2040/.*")
# Temporariamente removemos o HAL Wi-Fi (cyw43 ausente) para isolar o kernel
list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/hal/rp2350/hal_wifi_rp2350\\.cpp$")
# ==============================================================================

# ==============================================================================
# [CORREÇÃO LINKER] ADICIONAR FONTES FREERTOS MANUALMENTE
# Isso corrige os erros: cannot find -lFreeRTOS-Kernel e -lFreeRTOS-Kernel-Heap4
# ==============================================================================
list(APPEND SRC_FILES 
    "${FREERTOS_KERNEL_PATH}/tasks.c"
    "${FREERTOS_KERNEL_PATH}/queue.c"
    "${FREERTOS_KERNEL_PATH}/list.c"
    "${FREERTOS_KERNEL_PATH}/timers.c"
    "${FREERTOS_KERNEL_PATH}/event_groups.c"
    "${FREERTOS_KERNEL_PATH}/stream_buffer.c"
    "${FREERTOS_KERNEL_PATH}/portable/MemMang/heap_1.c"
)


# Remove files based on BUILD_TARGET option (Mantido original)
if(BUILD_TARGET STREQUAL "LVGL_DEMO")
    message(STATUS "Building LVGL RTOS demo")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/main\\.cpp$")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/tests/code_examples/qr_code_reader_demo\\.cpp$")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/tests/code_examples/fingerprint_demo\\.cpp$")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/tests/code_examples/sdcard_fatfs_demo\\.cpp$")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/tests/code_examples/wifi_demo\\.cpp$")
elseif(BUILD_TARGET STREQUAL "QR_DEMO")
    message(STATUS "Building QR Reader demo")
    # Exclui HAL Wi-Fi ao remover cyw43 para isolar o kernel
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/hal/rp2350/hal_wifi_rp2350\.cpp$")
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
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/main\\.cpp$")
    list(FILTER SRC_FILES EXCLUDE REGEX "${CMAKE_CURRENT_LIST_DIR}/src/core/state_machine\\.cpp$")
endif()


# -------------------------------
# Board-specific sources
# -------------------------------
if(BOARD STREQUAL "pico2_w")
    add_compile_definitions(BOARD_PICO2_W)
    add_compile_definitions(HAS_WIFI HAS_SD HAS_DISPLAY HAS_TOUCH)
    
    # [CORREÇÃO] Define plataforma explicitamente
    add_compile_definitions(RP2350_PLATFORM)

    # Nota: SRC_FILES já pegou o board_factory.cpp via GLOB, 
    # então não precisamos dar APPEND de novo para não duplicar.
    
    include_directories(
        src/boards
        src/boards/pico2_w
    )

    # Port específico para RP2350 (ARM NTZ)
    set(FREERTOS_PORT_PATH "${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2350_ARM_NTZ/non_secure")
    if(NOT EXISTS "${FREERTOS_PORT_PATH}/port.c")
        message(WARNING "RP2350 port ausente em ${FREERTOS_PORT_PATH}; usando fallback ARM_CM33_NTZ (menos estável no RP2350). Atualize o FreeRTOS-Kernel para incluir RP2350_ARM_NTZ.")
        set(FREERTOS_PORT_PATH "${FREERTOS_KERNEL_PATH}/portable/GCC/ARM_CM33_NTZ/non_secure")
    endif()

endif()

# Add executable. Default name is the project name, version 0.1
add_executable(${MAIN_TARGET_NAME}
        ${SRC_FILES}
        config/hw_config.c
)

pico_set_program_name(${MAIN_TARGET_NAME} "${MAIN_TARGET_NAME}")
pico_set_program_version(${MAIN_TARGET_NAME} "1.0.0")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(${MAIN_TARGET_NAME} 0)
pico_enable_stdio_usb(${MAIN_TARGET_NAME} 1)

# Definimos o Heap do C++ para 64KB (65536 bytes).
# NÃO colocar 4000000 aqui, pois isso é tamanho de FLASH, não de RAM.
# Em vez de usar função do SDK ou definição de compilador, passamos direto pro Linker.
# Isso define o Heap para 64KB (65536 bytes) de forma mandatória.
target_link_options(${PROJECT_NAME} PRIVATE "LINKER:--defsym=PICO_HEAP_SIZE=65536")

# Add the standard library to the build
target_link_libraries(${MAIN_TARGET_NAME}
        pico_stdlib
        )

# Add the standard include files to the build
target_include_directories(${MAIN_TARGET_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/src/config
        ${CMAKE_CURRENT_LIST_DIR}/lib
        ${CMAKE_CURRENT_LIST_DIR}/src
        ${CMAKE_CURRENT_LIST_DIR}/src/task
        ${CMAKE_CURRENT_LIST_DIR}/src/drivers
        ${CMAKE_CURRENT_LIST_DIR}/src/drivers/display
        ${CMAKE_CURRENT_LIST_DIR}/src/hal
        ${CMAKE_CURRENT_LIST_DIR}/src/hal/utils
        ${CMAKE_CURRENT_LIST_DIR}/src/hal/interfaces
        ${CMAKE_CURRENT_LIST_DIR}/src/ui/screens
        ${NO_OS_FATFS_PATH}/FatFs_SPI/include
        ${FREERTOS_PORT_PATH}
        ${FREERTOS_KERNEL_PATH}/include
)

# Adicionar os arquivos fontes do port FreeRTOS
if(BOARD STREQUAL "pico2_w")
    target_sources(${MAIN_TARGET_NAME} PRIVATE
        "${FREERTOS_PORT_PATH}/port.c"
        "${FREERTOS_PORT_PATH}/portasm.c"
    )
endif()

# Build a list of optional lwIP / wifi libs depending on FreeRTOS import
set(EXTRA_LWIP_LIBS)

if(FREERTOS_IMPORT_INCLUDED)
    list(APPEND EXTRA_LWIP_LIBS
    )
else()
    list(APPEND EXTRA_LWIP_LIBS
    )
endif()

# Add any user requested libraries
target_link_libraries(${MAIN_TARGET_NAME} 
        pico_stdlib
    hardware_exception
        hardware_i2c
        hardware_spi
        hardware_uart
        hardware_dma
        hardware_pwm
        hardware_gpio
        hardware_interp
        hardware_watchdog
        lvgl
        FatFs_SPI
        ${EXTRA_LWIP_LIBS}
        
        # [CORREÇÃO FINAL] REMOVIDO: FreeRTOS-Kernel e Heap4 
        # (Eles causavam o erro de linker porque agora são compilados via SRC_FILES)
)

target_compile_definitions(${MAIN_TARGET_NAME} PRIVATE)

pico_add_extra_outputs(${MAIN_TARGET_NAME})

# Link FreeRTOS heap/library only when targets exist (prevents unresolved -l<name> errors)
if(TARGET FreeRTOS-Kernel-Heap4)
    # target_link_libraries(${MAIN_TARGET_NAME} PRIVATE FreeRTOS-Kernel-Heap4)
elseif(TARGET FreeRTOS-Kernel)
    # target_link_libraries(${MAIN_TARGET_NAME} PRIVATE FreeRTOS-Kernel)
endif()