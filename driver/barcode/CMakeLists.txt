add_executable(barcode
        barcode.c
        )

target_link_libraries(barcode pico_stdlib hardware_adc)

# enable usb output, enable uart output
pico_enable_stdio_usb(barcode 1)
pico_enable_stdio_uart(barcode 1)

pico_add_extra_outputs(barcode)
# add url via pico_set_program_url
example_auto_set_url(barcode)

# create map/bin/hex file etc.
pico_add_extra_outputs(barcode)