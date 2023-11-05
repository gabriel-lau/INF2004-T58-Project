include(FreeRTOS_Kernel_import.cmake)

add_executable(main_sys
		main.c
		${PICO_LWIP_CONTRIB_PATH}/apps/ping/ping.c
		)
target_compile_definitions(main_sys PRIVATE
		WIFI_SSID=\"${WIFI_SSID}\"
		WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
		)
target_include_directories(main_sys PRIVATE
		${CMAKE_CURRENT_LIST_DIR}
		${CMAKE_CURRENT_LIST_DIR}/../.. # for our common lwipopts
		${PICO_LWIP_CONTRIB_PATH}/apps/ping
		)
target_link_libraries(main_sys
		pico_cyw43_arch_lwip_threadsafe_background
		pico_stdlib
		pico_lwip_iperf
		FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
		)
pico_enable_stdio_usb(main_sys 1)
pico_add_extra_outputs(main_sys)

add_executable(main_nosys
		main.c
		${PICO_LWIP_CONTRIB_PATH}/apps/ping/ping.c
		)
target_compile_definitions(main_nosys PRIVATE
		WIFI_SSID=\"${WIFI_SSID}\"
		WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
		NO_SYS=0            # don't want NO_SYS (generally this would be in your lwipopts.h)
		LWIP_SOCKET=1       # we need the socket API (generally this would be in your lwipopts.h)
		PING_USE_SOCKETS=1
		)
target_include_directories(main_nosys PRIVATE
		${CMAKE_CURRENT_LIST_DIR}
		${CMAKE_CURRENT_LIST_DIR}/../.. # for our common lwipopts
		${PICO_LWIP_CONTRIB_PATH}/apps/ping
		)
target_link_libraries(main_nosys
		pico_cyw43_arch_lwip_sys_freertos
		pico_stdlib
		pico_lwip_iperf
		FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
		)
pico_enable_stdio_usb(main_nosys 1)
pico_add_extra_outputs(main_nosys)
