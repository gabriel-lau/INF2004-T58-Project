#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
// Include necessary libraries
// #include "FreeRTOS.h"
// #include "task.h"
// #include "queue.h"
// #include "semphr.h"
// Include headers
#include "lwipopts.h"
#include "http_server.h"
#include "tcp_server.h"

// FreeRTOS
#ifndef RUN_FREERTOS_ON_CORE
#define RUN_FREERTOS_ON_CORE 0
#endif

// Define task handles and other global variables
// TaskHandle_t httpServerTaskHandle;
// TaskHandle_t tcpServerTaskHandle;
// SemaphoreHandle_t resourceMutex;

// // TCP server task
// void TCPServerTask(void* pvParameters) {
//     // Initialize and run the TCP server           
//     run_tcp_server();
   
//     while (1) {    
//         vTaskDelay(pdMS_TO_TICKS(100)); // Delay to yield to other tasks
//         // Your TCP server code here
//         // Use xSemaphoreTake to access shared resources protected by resourceMutex
//         // Listen for incoming connections
//         // Handle incoming data
//         // Send responses
//     }
// }
// // HTTP server task
// void HTTPServerTask(void* pvParameters) {
//     // Initialize and run the HTTP server        
//     run_http_server();

//     while (1) {
//         // Your HTTP server code here
//         // Use xSemaphoreTake to access shared resources protected by resourceMutex
//         // Handle HTTP requests
//         // Send responses

//         vTaskDelay(pdMS_TO_TICKS(100)); // Delay to yield to other tasks
//         // fgets(console_input, sizeof(console_input), stdin);
//         // printf("entered: %s", console_input);
//     }
// }


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
    printf("Connected to WiFi! \n\n");

    // Create a mutex to protect shared resources
    // resourceMutex = xSemaphoreCreateMutex();
    
    // Create the HTTP server task
    // xTaskCreate(HTTPServerTask, "HTTP Server Task", configMINIMAL_STACK_SIZE, NULL, 2, &httpServerTaskHandle);
    printf("Created HTTP Server Task\n");
    run_http_server();

    // Create the TCP server task
    // xTaskCreate(TCPServerTask, "TCP Server Task", configMINIMAL_STACK_SIZE, NULL, 3, &tcpServerTaskHandle);
    printf("Created TCP Server Task\n");
    run_tcp_server();


    // Start the FreeRTOS scheduler
    // vTaskStartScheduler();

    // The scheduler will now take over, and tasks will execute    
    for (;;);
    // while (1) {
    //     // Your code may not reach this point
    // }
    // return 0;
}
