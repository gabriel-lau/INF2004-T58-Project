cmake_minimum_required(VERSION 3.13)

set(PROGRAM_NAME pico_w_webserver)
set(PICO_BOARD pico_w)

include(../pico_sdk_import.cmake)

project(pico_w_webserver)

pico_sdk_init()

message("Running makefsdata.py script")
execute_process(COMMAND
    py makefsdata.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)
message("Created fsdata.c")

set(PROGRAM_NAME pico_w_webserver)

if (DEFINED ENV{WIFI_SSID} AND (NOT WIFI_SSID))
    set(WIFI_SSID $ENV{WIFI_SSID})
    message("Using WIFI_SSID from environment ('${WIFI_SSID}')")
endif()

if (DEFINED ENV{WIFI_PASSWORD} AND (NOT WIFI_PASSWORD))
    set(WIFI_PASSWORD $ENV{WIFI_PASSWORD})
    message("Using WIFI_PASSWORD from environment")
endif()

set(WIFI_SSID "${WIFI_SSID}" CACHE INTERNAL "WiFi SSID for examples")
set(WIFI_PASSWORD "${WIFI_PASSWORD}" CACHE INTERNAL "WiFi password for examples")

add_executable(${PROGRAM_NAME}
    server.c
)
target_compile_definitions(project PRIVATE
    WIFI_SSID=\"${WIFI_SSID}\"
    WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
)

target_include_directories(${PROGRAM_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# pull in common dependencies
target_link_libraries(${PROGRAM_NAME}
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_http
    pico_stdlib                                     # for core functionality
    hardware_adc
)

# enable usb output, disable uart output
pico_enable_stdio_usb(${PROGRAM_NAME} 1)
pico_enable_stdio_uart(${PROGRAM_NAME} 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(${PROGRAM_NAME})

# add url via pico_set_program_url
example_auto_set_url(${PROGRAM_NAME})

# if (PICO_CYW43_SUPPORTED) # set by PICO_BOARD=pico_w
#     if (TARGET pico_cyw43_arch)
#         message("Skipping Pico W examples as support is not available")

#         if (DEFINED ENV{WIFI_SSID} AND (NOT WIFI_SSID))
#             set(WIFI_SSID $ENV{WIFI_SSID})
#             message("Using WIFI_SSID from environment ('${WIFI_SSID}')")
#         endif()

#         if (DEFINED ENV{WIFI_PASSWORD} AND (NOT WIFI_PASSWORD))
#             set(WIFI_PASSWORD $ENV{WIFI_PASSWORD})
#             message("Using WIFI_PASSWORD from environment")
#         endif()

#         set(WIFI_SSID "${WIFI_SSID}" CACHE INTERNAL "WiFi SSID for examples")
#         set(WIFI_PASSWORD "${WIFI_PASSWORD}" CACHE INTERNAL "WiFi password for examples")


#         add_executable(project
#             project.c
#         )
#         target_compile_definitions(project PRIVATE
#                 WIFI_SSID=\"${WIFI_SSID}\"
#                 WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
#                 )
#         # target_include_directories(project PRIVATE
#         #         ${CMAKE_CURRENT_LIST_DIR}
#         #         ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
#         #         )
#         target_link_libraries(
#                 project
#                 pico_stdlib
#                 pico_cyw43_arch
#                 )

#         pico_add_extra_outputs(project)
#         example_auto_set_url(project)
#         # add_subdirectory(wifi)
        
#         # if (TARGET pico_btstack_base)
#         #     add_subdirectory(bt)
#         # endif()
#     endif()
# endif()
