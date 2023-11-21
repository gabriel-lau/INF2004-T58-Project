#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

// Include headers
#include "wifi.h"
#include "http_server.h"
#include "tcp_server.h"

void wifiSetup()
{
    // Create the HTTP server task
    printf("Created HTTP Server Task\n");
    run_http_server();

    // Create the TCP server task
    printf("Created TCP Server Task\n");
    run_tcp_server();
}

void testCase1()
{
    // Create the test task
    // printf("Started On\n");
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
}

void testCase2()
{
    // Create the test task
    // printf("Started Off\n");
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
}