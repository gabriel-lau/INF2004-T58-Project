#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwipopts.h"

// Include headers
#include "wifi.h"
#include "http_server.h"
#include "tcp_server.h"

void wifiSetup()
{
    // stdio_init_all();

    if (cyw43_arch_init()) {
        printf("Failed to initialise\n");
        // return 1;
    }

    // use the predefined `cyw43_pm_value` macro:
    cyw43_arch_enable_sta_mode();
    // it doesn't use the `CYW43_NO_POWERSAVE_MODE` value, so we do this instead:
    cyw43_wifi_pm(&cyw43_state, cyw43_pm_value(CYW43_NO_POWERSAVE_MODE, 20, 1, 1, 1));

    // Connect to the WiFI network - loop until connected
    while(cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000) != 0){
        printf("Connecting to WiFi...\n");
    }
    // Print a success message once connected
    printf("Connected to WiFi! \n\n");
    
    // Create the HTTP server task
    printf("Created HTTP Server Task\n");
    run_http_server();

    // Create the TCP server task
    printf("Created TCP Server Task\n");
    run_tcp_server();
}
