cmake_minimum_required(VERSION 3.12)

# Add your source file
add_executable(magnometer
    magnometer.c
)

# Include Pico SDK
pico_enable_stdio_usb(magnometer 1)
pico_enable_stdio_uart(magnometer 0)

# Link with Pico SDK libraries
target_link_libraries(magnometer PRIVATE pico_stdlib hardware_gpio hardware_i2c)


pico_add_extra_outputs(magnometer)
