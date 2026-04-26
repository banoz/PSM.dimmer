cmake_minimum_required(VERSION 3.20)

project(psm_dimmer LANGUAGES C ASM)

set(TARGET_NAME psm_dimmer CACHE STRING "Firmware target name")

if(DEFINED ENV{WCH_SDK} AND NOT "$ENV{WCH_SDK}" STREQUAL "")
    set(DEFAULT_WCH_SDK "$ENV{WCH_SDK}")
else()
    set(DEFAULT_WCH_SDK "/opt/wch-sdk")
endif()

set(WCH_SDK "${DEFAULT_WCH_SDK}" CACHE PATH "Path to the WCH CH32V003 EVT package root")

if(NOT CMAKE_OBJCOPY)
    find_program(CMAKE_OBJCOPY NAMES riscv-none-elf-objcopy REQUIRED)
endif()

if(NOT CMAKE_SIZE)
    find_program(CMAKE_SIZE NAMES riscv-none-elf-size REQUIRED)
endif()

set(DRV_DIR "${WCH_SDK}/EVT/EXAM/SRC/Peripheral")
set(STARTUP_DIR "${WCH_SDK}/EVT/EXAM/SRC/Startup")

if(NOT EXISTS "${DRV_DIR}/inc/ch32v00x.h")
    message(FATAL_ERROR "WCH SDK not found at ${WCH_SDK}. Set WCH_SDK to your CH32V003EVT path.")
endif()

set(APP_SRCS
    src/main.c
    src/psm.c
)

set(DRV_SRCS
    ${DRV_DIR}/src/ch32v00x_adc.c
    ${DRV_DIR}/src/ch32v00x_dbgmcu.c
    ${DRV_DIR}/src/ch32v00x_dma.c
    ${DRV_DIR}/src/ch32v00x_exti.c
    ${DRV_DIR}/src/ch32v00x_flash.c
    ${DRV_DIR}/src/ch32v00x_gpio.c
    ${DRV_DIR}/src/ch32v00x_i2c.c
    ${DRV_DIR}/src/ch32v00x_iwdg.c
    ${DRV_DIR}/src/ch32v00x_misc.c
    ${DRV_DIR}/src/ch32v00x_opa.c
    ${DRV_DIR}/src/ch32v00x_pwr.c
    ${DRV_DIR}/src/ch32v00x_rcc.c
    ${DRV_DIR}/src/ch32v00x_spi.c
    ${DRV_DIR}/src/ch32v00x_tim.c
    ${DRV_DIR}/src/ch32v00x_usart.c
    ${DRV_DIR}/src/ch32v00x_wwdg.c
)

set(SYS_SRCS
    ${STARTUP_DIR}/core_riscv.c
    ${STARTUP_DIR}/system_ch32v00x.c
)

set(STARTUP_SRC
    ${STARTUP_DIR}/startup_ch32v00x.S
)

add_executable(${TARGET_NAME}
    ${APP_SRCS}
    ${DRV_SRCS}
    ${SYS_SRCS}
    ${STARTUP_SRC}
)

set_target_properties(${TARGET_NAME} PROPERTIES SUFFIX ".elf")

target_compile_definitions(${TARGET_NAME} PRIVATE
    CH32V00x
    USE_STDPERIPH_DRIVER
)

target_include_directories(${TARGET_NAME} PRIVATE
    src
    ${DRV_DIR}/inc
    ${STARTUP_DIR}
)

target_compile_options(${TARGET_NAME} PRIVATE
    -march=rv32ec
    -mabi=ilp32e
    $<$<COMPILE_LANGUAGE:C>:-Os>
    $<$<COMPILE_LANGUAGE:C>:-g3>
    $<$<COMPILE_LANGUAGE:C>:-ffunction-sections>
    $<$<COMPILE_LANGUAGE:C>:-fdata-sections>
    $<$<COMPILE_LANGUAGE:C>:-fno-common>
    $<$<COMPILE_LANGUAGE:C>:-Wall>
    $<$<COMPILE_LANGUAGE:C>:-Wextra>
)

target_link_options(${TARGET_NAME} PRIVATE
    -march=rv32ec
    -mabi=ilp32e
    -nostartfiles
    -T${CMAKE_CURRENT_SOURCE_DIR}/ld/ch32v003j4m6.ld
    -Wl,--gc-sections
    -Wl,-Map,${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.map
)

add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${TARGET_NAME}> ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${TARGET_NAME}> ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${TARGET_NAME}>
    BYPRODUCTS
        ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.hex
        ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.bin
    COMMENT "Generating HEX/BIN artifacts and reporting size"
)