cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(pico_freertos_blink C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_definitions(LIB_CMSIS_CORE=1)

pico_sdk_init()

# FreeRTOS kernel sources
set(FREERTOS_KERNEL_PATH "${CMAKE_CURRENT_LIST_DIR}/lib/FreeRTOS-Kernel")

add_library(freertos_kernel
    ${FREERTOS_KERNEL_PATH}/croutine.c
    ${FREERTOS_KERNEL_PATH}/event_groups.c
    ${FREERTOS_KERNEL_PATH}/list.c
    ${FREERTOS_KERNEL_PATH}/queue.c
    ${FREERTOS_KERNEL_PATH}/stream_buffer.c
    ${FREERTOS_KERNEL_PATH}/tasks.c
    ${FREERTOS_KERNEL_PATH}/timers.c
    ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/port.c
    ${FREERTOS_KERNEL_PATH}/portable/MemMang/heap_4.c
)

target_include_directories(freertos_kernel PUBLIC
    ${FREERTOS_KERNEL_PATH}/include
    ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040
    ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/include
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(freertos_kernel
    pico_platform
    pico_sync
    pico_time
    pico_stdlib
    pico_multicore
    hardware_exception
    hardware_sync
    hardware_spi
    hardware_dma
    hardware_claim
    pico_base_headers
)

target_compile_definitions(freertos_kernel PUBLIC
    PICO_RP2040=1
    PICO_DEFAULT_UART=0                 
    PICO_DEFAULT_UART_TX_PIN=0           
    PICO_DEFAULT_UART_RX_PIN=1           
    PICO_DEFAULT_UART_BAUD_RATE=115200   
)

# Main executable
add_executable(main
    src/main.c
    ./src/uart_task.c
    ./src/emulate.c
    src/usb_descriptors.c
)

target_link_libraries(main
    freertos_kernel
    pico_stdlib
    pico_time
    tinyusb_device
    tinyusb_board
)

pico_enable_stdio_usb(main 0)
pico_enable_stdio_uart(main 1)
pico_enable_stdio_semihosting(main 0)

pico_add_extra_outputs(main)
