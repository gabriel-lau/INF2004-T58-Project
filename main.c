/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <math.h>

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"
#include "queue.h"

#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/timer.h"
#include "hardware/pwm.h"

#include "driver/motor/motor.c"

#define mbaTASK_MESSAGE_BUFFER_SIZE       ( 60 )

#ifndef RUN_FREERTOS_ON_CORE
#define RUN_FREERTOS_ON_CORE 0
#endif

#define TEST_TASK_PRIORITY				( tskIDLE_PRIORITY + 1UL )

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
    //Init Left GPIO
    gpio_init(INPUT_1_LEFT);
    gpio_init(INPUT_2_LEFT);
    gpio_init(PWM_LEFT);

    //Init Right GPIO
    gpio_init(INPUT_1_RIGHT);
    gpio_init(INPUT_2_RIGHT);
    gpio_init(PWM_RIGHT);

    //Set Left GPIO to out power board
    gpio_set_dir(INPUT_1_LEFT, GPIO_OUT);
    gpio_set_dir(INPUT_2_LEFT, GPIO_OUT);
    gpio_set_function(PWM_LEFT, GPIO_FUNC_PWM);

    //Set Right GPIO to out power board
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
        vTaskDelay(1000);
    }
}

void vLaunch( void) {

    TaskHandle_t motorTaskHandle;
    xTaskCreate(motorTask, "TestTempThread", configMINIMAL_STACK_SIZE, NULL, 8, &motorTaskHandle);


#if NO_SYS && configUSE_CORE_AFFINITY && configNUM_CORES > 1
    // we must bind the main task to one core (well at least while the init is called)
    // (note we only do this in NO_SYS mode, because cyw43_arch_freertos
    // takes care of it otherwise)
    vTaskCoreAffinitySet(task, 1);
#endif

    /* Start the tasks and timer running. */
    vTaskStartScheduler();
}

void motorSetup()
{
    
}

int main( void )
{
    stdio_init_all();
    void motorSetup();
    /* Configure the hardware ready to run the demo. */
    const char *rtos_name;
#if ( portSUPPORT_SMP == 1 )
    rtos_name = "FreeRTOS SMP";
#else
    rtos_name = "FreeRTOS";
#endif

#if ( portSUPPORT_SMP == 1 ) && ( configNUM_CORES == 2 )
    printf("Starting %s on both cores:\n", rtos_name);
    vLaunch();
#elif ( RUN_FREERTOS_ON_CORE == 1 )
    printf("Starting %s on core 1:\n", rtos_name);
    multicore_launch_core1(vLaunch);
    while (true);
#else
    printf("Starting %s on core 0:\n", rtos_name);
    vLaunch();
#endif
    return 0;
}
