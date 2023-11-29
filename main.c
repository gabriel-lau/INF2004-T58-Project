/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"
#include "queue.h"

#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "hardware/timer.h"

#include "driver/motor/motor.c"

#include "driver/wifi/wifi.h"
#include "driver/wifi/tcp_server.h"

#include "driver/irline/barcode/barcode.h"

#define mbaTASK_MESSAGE_BUFFER_SIZE (60)

#ifndef RUN_FREERTOS_ON_CORE
#define RUN_FREERTOS_ON_CORE 0
#endif

#define TEST_TASK_PRIORITY (tskIDLE_PRIORITY + 1UL)

// Message buffer handle
static MessageBufferHandle_t xBarcodeMessageBuffer;

// Define Tasks
TaskHandle_t motorTaskHandle;
TaskHandle_t wifiTaskHandle;
TaskHandle_t barcodeTaskHandle;

// GPIO pins for MOTOR
int INPUT_1_LEFT = 12;
int INPUT_2_LEFT = 13;
int PWM_LEFT = 14;
int INPUT_1_RIGHT = 19;
int INPUT_2_RIGHT = 18;
int PWM_RIGHT = 17;
uint16_t PWM_LEFT_CYCLE = 32768;
uint16_t PWM_RIGHT_CYCLE = 32768;

// GPIO pins for ENCODER
const int ENCODER_LEFT = 15;
const int ENCODER_RIGHT = 16;

// GPIO pins for MAGNOMETER
const int MAG_SDA = 0;
const int MAG_SCL = 1;

// GPIO pins for ULTRASONIC
const int TRIGGER_PIN = 2;
const int ECHO_PIN = 3;

// GPIO pins for IR

void motorTask(void *pvParameters)
{
    // Init Left GPIO
    gpio_init(INPUT_1_LEFT);
    gpio_init(INPUT_2_LEFT);
    gpio_init(PWM_LEFT);

    // Init Right GPIO
    gpio_init(INPUT_1_RIGHT);
    gpio_init(INPUT_2_RIGHT);
    gpio_init(PWM_RIGHT);

    // Set Left GPIO to out power board
    gpio_set_dir(INPUT_1_LEFT, GPIO_OUT);
    gpio_set_dir(INPUT_2_LEFT, GPIO_OUT);
    gpio_set_function(PWM_LEFT, GPIO_FUNC_PWM);

    // Set Right GPIO to out power board
    gpio_set_dir(INPUT_1_RIGHT, GPIO_OUT);
    gpio_set_dir(INPUT_2_RIGHT, GPIO_OUT);
    gpio_set_function(PWM_RIGHT, GPIO_FUNC_PWM);

    // Find out which PWM slice is connected to GPIO
    uint slice_num_left = pwm_gpio_to_slice_num(PWM_LEFT);
    uint slice_num_right = pwm_gpio_to_slice_num(PWM_RIGHT);

    pwm_set_gpio_level(PWM_LEFT, PWM_LEFT_CYCLE);
    pwm_set_gpio_level(PWM_RIGHT, PWM_RIGHT_CYCLE);

    pwm_set_enabled(slice_num_left, true);
    pwm_set_enabled(slice_num_right, true);
    while (1)
    {
        moveForward();
        /*
        vTaskDelay(1000);
        stop();
        vTaskDelay(1000);
        moveBackward();
        vTaskDelay(1000);
        stop();
        vTaskDelay(1000);
        turnLeft();
        vTaskDelay(1000);
        stop();
        vTaskDelay(1000);*/
    }
}

void barcodeTask(void *pvParameters)
{
    printf("Initializing barcode reader\n");

    // Initialize ADC for reading barcode sensor data
    adc_init();
    adc_gpio_init(BARCODE_SENSOR_PIN);
    adc_select_input(BARCODE_ADC_CHANNEL);

    // Reset reading parameters to initial state
    resetReadingParameters();
    resetCheckParameters();

    // Enable GPIO interrupt for wall sensor
    gpio_set_irq_enabled_with_callback(WALL_SENSOR_PIN, GPIO_IRQ_EDGE_RISE, true, &handleSensorInterrupt);

    while (true)
    {
        if (decoded_complete)
        {
            if (decoded_char != '\0')
            {
                printf("Barcode complete: %i\n", decoded_complete);
                printf("Barcode char: %c\n\n", decoded_char);

                decoded_complete = false;
                // Convert float to JSON string
                char barcodeBuffer[mbaTASK_MESSAGE_BUFFER_SIZE]; // Adjust the size based on your needs
                snprintf(barcodeBuffer, sizeof(barcodeBuffer), "{\"barcode\": \"%c\"}", decoded_char);

                // Send JSON data to wifiTask
                xMessageBufferSend(
                    xBarcodeMessageBuffer,     /* The message buffer to write to. */
                    (void *)barcodeBuffer,     /* The source of the data to send. */
                    strlen(barcodeBuffer) + 1, /* Include null-terminator in length */
                    0);                        /* Do not block, should the buffer be full. */
            }
            resetCheckParameters();
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
};

void wifiTask(void *pvParameters)
{
    // const char *receivedData;
    char receivedData[mbaTASK_MESSAGE_BUFFER_SIZE];

    if (cyw43_arch_init())
    {
        printf("Failed to initialise\n");
        // return 1;
    }

    // use the predefined `cyw43_pm_value` macro:
    cyw43_arch_enable_sta_mode();
    // it doesn't use the `CYW43_NO_POWERSAVE_MODE` value, so we do this instead:
    cyw43_wifi_pm(&cyw43_state, cyw43_pm_value(CYW43_NO_POWERSAVE_MODE, 20, 1, 1, 1));

    // Connect to the WiFI network - loop until connected
    while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000) != 0)
    {
        printf("Connecting to WiFi...\n");
    }
    // Print a success message once connected
    printf("Connected to WiFi! \n\n");

    wifiSetup();

    while (1)
    {
#if PICO_CYW43_ARCH_POLL
        // if you are using pico_cyw43_arch_poll, then you must poll periodically from your
        // main loop (not from a timer) to check for Wi-Fi driver or lwIP work that needs to be done.
        cyw43_arch_poll();
        // you can poll as often as you like, however if you have nothing else to do you can
        // choose to sleep until either a specified time, or cyw43_arch_poll() has work to do:
        cyw43_arch_wait_for_work_until(make_timeout_time_ms(1000));
#else
        // if you are not using pico_cyw43_arch_poll, then WiFI driver and lwIP work
        // is done via interrupt in the background. This sleep is just an example of some (blocking)
        // work you might be doing.
        sleep_ms(1000);
#endif
        vTaskDelay(1000);

        // Read data from the message buffer
        size_t bytesRead = xMessageBufferReceive(xBarcodeMessageBuffer, receivedData, sizeof(receivedData) - 1, portMAX_DELAY);

        // Check if data was received
        if (bytesRead > 0)
        {
            // Null-terminate the received data
            receivedData[bytesRead] = '\0';

            // Print the received data
            printf("Received message buffer. Received data: %s\n", receivedData);

            // Send data to the TCP server
            send_message(receivedData);
        }
        else
        {
            printf("Failed to receive message buffer. Received data: %s\n", receivedData);
        }
    };
}

void vLaunch(void)
{
    initialize_mutex();

    // Motor Task handle
    xTaskCreate(motorTask, "MotorTask", configMINIMAL_STACK_SIZE, NULL, 1, &motorTaskHandle);

    // Wifi Task handle
    xTaskCreate(wifiTask, "WifiThread", configMINIMAL_STACK_SIZE, NULL, 1, &wifiTaskHandle);

    // Barcode Task handle
    xTaskCreate(barcodeTask, "BarcodeThread", configMINIMAL_STACK_SIZE, NULL, 1, &barcodeTaskHandle);

    // Create the message buffer
    xBarcodeMessageBuffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);

#if NO_SYS && configUSE_CORE_AFFINITY && configNUM_CORES > 1
    // we must bind the main task to one core (well at least while the init is called)
    // (note we only do this in NO_SYS mode, because cyw43_arch_freertos
    // takes care of it otherwise)
    vTaskCoreAffinitySet(task, 1);
#endif
    /* Start the tasks and timer running. */
    vTaskStartScheduler();
}

int main(void)
{
    stdio_init_all();

    /* Configure the hardware ready to run the demo. */
    const char *rtos_name;
#if (portSUPPORT_SMP == 1)
    rtos_name = "FreeRTOS SMP";
#else
    rtos_name = "FreeRTOS";
#endif

#if (portSUPPORT_SMP == 1) && (configNUM_CORES == 2)
    printf("Starting %s on both cores:\n", rtos_name);
    vLaunch();
#elif (RUN_FREERTOS_ON_CORE == 1)
    printf("Starting %s on core 1:\n", rtos_name);
    multicore_launch_core1(vLaunch);
    while (true)
        ;
#else
    printf("Starting %s on core 0:\n", rtos_name);
    vLaunch();
#endif
    return 0;
}