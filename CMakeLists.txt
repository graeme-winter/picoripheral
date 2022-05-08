cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_SDK_PATH "/home/pi/C/pico/pico-sdk")

include(pico_sdk_import.cmake)

project(picoripheral C CXX ASM)

pico_sdk_init()

add_executable(picoripheral picoripheral.c)

pico_set_program_name(picoripheral "picoripheral")
pico_set_program_version(picoripheral "0.1")

# run from RAM for performance reasons (will be reacting to interrupts)
pico_set_binary_type(picoripheral no_flash)

# stdout -> UART
pico_enable_stdio_uart(picoripheral 1)
pico_enable_stdio_usb(picoripheral 0)

target_link_libraries(picoripheral pico_stdlib)
target_link_libraries(picoripheral hardware_i2c)

pico_add_extra_outputs(picoripheral)
