cmake_minimum_required(VERSION 3.12)
add_subdirectory(barcode)

# Add your source file
add_executable(irline
    irline_driver_demo.c
)

# Include Pico SDK
pico_enable_stdio_usb(irline 1)
pico_enable_stdio_uart(irline 0)

target_link_libraries(irline PRIVATE pico_stdlib hardware_gpio)

pico_add_extra_outputs(irline)
