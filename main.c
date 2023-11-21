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

#include "driver/motor/motor.h"
#include "driver/ultrasonic/ultrasonic.h"
#include "driver/irline/irline.h"

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
int ENCODER_LEFT = 15;
int ENCODER_RIGHT = 16;

// GPIO pins for MAGNOMETER
const int MAG_SDA = 0;
const int MAG_SCL = 1;

// GPIO pins for ULTRASONIC
const int TRIGGER_PIN = 2;
const int ECHO_PIN = 3;

// GPIO pins for IR
const uint IR_PIN_RIGHT = 4;
const uint IR_PIN_LEFT = 5;

// get direction by distance
void setDir(int distance) // change direction if meet obstacle
{
    if (distance <= 5)
    {
        moveBackward();
        printf("%d",distance);
        printf("Stop\n");
    }
    else 
    {
        moveForward();
        printf("Forward\n");
    }
}

void motorTask(void *pvParameters)
{
    gpio_init(IR_PIN_LEFT);
    gpio_set_dir(IR_PIN_LEFT, GPIO_IN);
    gpio_init(IR_PIN_RIGHT);
    gpio_set_dir(IR_PIN_RIGHT, GPIO_IN);

    motorSetup();
    while (1)
    {   
        moveForward();
        /*
        printf("LEFT IS HERE\n");
        irLine(IR_PIN_LEFT);
        printf("RIGHT IS HERE\n");
        irLine(IR_PIN_RIGHT);
        setupUltrasonicPins(TRIGGER_PIN, ECHO_PIN);
        getCm(TRIGGER_PIN, ECHO_PIN);
        setDir(getDistance());
        vTaskDelay(1000);
        printf("\n");*/
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

int main( void )
{
    stdio_init_all();
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
