set(PROGRAM_NAME barcode)

# Add your source files
add_executable(${PROGRAM_NAME}
    barcode.c
)

target_compile_definitions(${PROGRAM_NAME} PRIVATE
    PING_USE_SOCKETS=1
)

target_include_directories(${PROGRAM_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# pull in common dependencies
target_link_libraries(${PROGRAM_NAME}
    pico_cyw43_arch_lwip_sys_freertos
    pico_stdlib                                     # for core functionality
    hardware_adc
    hardware_pwm
    pico_lwip_iperf
    FreeRTOS-Kernel-Heap4
)

# enable usb output, disable uart output
pico_enable_stdio_usb(${PROGRAM_NAME} 1)
pico_enable_stdio_uart(${PROGRAM_NAME} 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(${PROGRAM_NAME})

# add url via pico_set_program_url
example_auto_set_url(${PROGRAM_NAME})
