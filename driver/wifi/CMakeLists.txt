# set(PROGRAM_NAME wifi)

# # message("Running makefsdata.py script")
# # execute_process(COMMAND
# #     py makefsdata.py
# #     WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
# # )
# # message("Created makefsdata.c")

# if (DEFINED ENV{WIFI_SSID} AND (NOT WIFI_SSID))
#     set(WIFI_SSID $ENV{WIFI_SSID})
#     message("Using WIFI_SSID from environment ('${WIFI_SSID}')")
# endif()

# if (DEFINED ENV{WIFI_PASSWORD} AND (NOT WIFI_PASSWORD))
#     set(WIFI_PASSWORD $ENV{WIFI_PASSWORD})
#     message("Using WIFI_PASSWORD from environment")
# endif()

# set(WIFI_SSID "${WIFI_SSID}" CACHE INTERNAL "WiFi SSID for examples")
# set(WIFI_PASSWORD "${WIFI_PASSWORD}" CACHE INTERNAL "WiFi password for examples")

# add_executable(${PROGRAM_NAME}
#     wifi.c
#     http_server.c
#     tcp_server.c    
# )

# target_compile_definitions(${PROGRAM_NAME} PRIVATE
#     WIFI_SSID=\"${WIFI_SSID}\"
#     WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
#     # PING_USE_SOCKETS=1
# )

# target_include_directories(${PROGRAM_NAME} PRIVATE
#     ${CMAKE_CURRENT_LIST_DIR}
# )

# # pull in common dependencies
# target_link_libraries(${PROGRAM_NAME}
#     # pico_cyw43_arch_lwip_sys_freertos
#     pico_cyw43_arch_lwip_threadsafe_background
#     pico_lwip_http
#     pico_stdlib                                     # for core functionality
#     hardware_adc
#     # pico_lwip_iperf
#     # FreeRTOS-Kernel-Heap4
# )

# # enable usb output, disable uart output
# pico_enable_stdio_usb(${PROGRAM_NAME} 1)
# pico_enable_stdio_uart(${PROGRAM_NAME} 0)

# # create map/bin/hex file etc.
# pico_add_extra_outputs(${PROGRAM_NAME})

# # add url via pico_set_program_url
# example_auto_set_url(${PROGRAM_NAME})