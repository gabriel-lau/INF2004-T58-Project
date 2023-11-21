cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

include(pico_extras_import_optional.cmake)

project(pico_examples C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_BOARD pico_w)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

# Initialize the SDK
pico_sdk_init()

include(example_auto_set_url.cmake)

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        )
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-maybe-uninitialized)
endif()

# Hardware-specific examples in subdirectories:
#add_subdirectory(main)
add_subdirectory(driver)

include(FreeRTOS_Kernel_import.cmake)

#######################################################################################################################################
########################################################## CMAKE MAIN.c file ##########################################################
#######################################################################################################################################
add_executable(main
        main.c
        #${PICO_LWIP_CONTRIB_PATH}/apps/ping/ping.c
        )
target_compile_definitions(main PRIVATE
        WIFI_SSID=\"${WIFI_SSID}\"
        WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
        NO_SYS=0  
        )

target_include_directories(main PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        #${CMAKE_CURRENT_LIST_DIR}/../.. # for our common lwipopts
        #${PICO_LWIP_CONTRIB_PATH}/apps/ping
        )

target_link_libraries(main
        pico_cyw43_arch_lwip_sys_freertos
        pico_stdlib
        pico_lwip_iperf
        hardware_adc
		hardware_pwm
        FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
        hardware_i2c
        )
pico_enable_stdio_usb(main 1)
pico_add_extra_outputs(main)

# target_link_libraries(magnometer PRIVATE pico_stdlib hardware_gpio hardware_i2c)
