#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/apps/httpd.h"
#include "lwipopts.h"
#include "http_server.h"
#include "ssi.h"
#include "cgi.h"

// WIFI Credentials - take care if pushing to github!
// const char WIFI_SSID[] = "";
// const char WIFI_PASSWORD[] = "";

void run_http_server() {
    // Initialise web server
    httpd_init();
    extern cyw43_t cyw43_state;
    int ip_addr = cyw43_state.netif[CYW43_ITF_STA].ip_addr.addr;
    printf("HTTP Server initialised\n");
    printf("Starting http server at %lu.%lu.%lu.%lu\n\n", ip_addr & 0xFF, (ip_addr >> 8) & 0xFF, (ip_addr >> 16) & 0xFF, ip_addr >> 24);

    // Configure SSI and CGI handler
    ssi_init(); 
    printf("SSI Handler initialised\n");
    cgi_init();
    printf("CGI Handler initialised\n");
    printf("\n");
}