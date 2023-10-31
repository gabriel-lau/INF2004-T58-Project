#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "lwip/apps/httpd.h"
#include "pico/cyw43_arch.h"
#include "lwipopts.h"
#include "ssi.h"
#include "cgi.h"

// WIFI Credentials - take care if pushing to github!
// const char WIFI_SSID[] = "";
// const char WIFI_PASSWORD[] = "";

int main() {
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("Failed to initialise\n");
        return 1;
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
    printf("Connected to WiFi! \n");
    extern cyw43_t cyw43_state;
    int ip_addr = cyw43_state.netif[CYW43_ITF_STA].ip_addr.addr;
    printf("IP Address: %lu.%lu.%lu.%lu\n", ip_addr & 0xFF, (ip_addr >> 8) & 0xFF, (ip_addr >> 16) & 0xFF, ip_addr >> 24);

    // Initialise web server
    httpd_init();
    printf("HTTP Server initialised\n");

    // Configure SSI and CGI handler
    ssi_init(); 
    printf("SSI Handler initialised\n");
    cgi_init();
    printf("CGI Handler initialised\n");
    printf("\n");

    // Infinite loop
    // for (;;);
    while (1){
        fgets(console_input, sizeof(console_input), stdin);
        printf("entered: %s", console_input);
    };  

}