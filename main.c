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
#include "driver/magnometer/magnometer.h"

#define mbaTASK_MESSAGE_BUFFER_SIZE       ( 60 )

#ifndef RUN_FREERTOS_ON_CORE
#define RUN_FREERTOS_ON_CORE 0
#endif

#define TEST_TASK_PRIORITY				( tskIDLE_PRIORITY + 1UL )

static QueueHandle_t xControlQueue;

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
int TRIG_PIN = 2;
int ECHO_PIN = 3;

// GPIO pins for IR
const uint IR_PIN_RIGHT = 4;
const uint IR_PIN_LEFT = 5;

// get direction by distance
void setDir(int distance) // change direction if meet obstacle
{
    printf("Distance: %d cm\n", distance);
    if (distance <= 15)
    {
        moveBackward();
        //printf("%d",distance);
        //printf("Stop\n");
        xQueueSend(xControlQueue, "s", portMAX_DELAY);
    }
    else 
    {
        moveForward();
        printf("Forward\n");
        xQueueSend(xControlQueue, "f", portMAX_DELAY);
    }
}

void motorTask(void *pvParameters)
{
    gpio_init(IR_PIN_LEFT);
    gpio_set_dir(IR_PIN_LEFT, GPIO_IN);
    gpio_init(IR_PIN_RIGHT);
    gpio_set_dir(IR_PIN_RIGHT, GPIO_IN);

    motorSetup();
    char xReceivedChar;
    size_t xReceivedBytes;
    while (1)
    {   
        xReceivedBytes = xQueueReceive(xControlQueue, &xReceivedChar, portMAX_DELAY);
        printf("Received %c\n", xReceivedChar);
        if  (xReceivedChar == 'f')
        {
            moveForward();
        }
        else if (xReceivedChar == 's')
        {
            moveBackward();
        }        
        if(irLine(IR_PIN_LEFT) == 1){
            turnRight();
            printf("Turn Right\n");
        }
        printf("RIGHT IS HERE\n");
        irLine(IR_PIN_RIGHT);
        if(irLine(IR_PIN_RIGHT) == 1){
            turnLeft();
            printf("Turn Left\n");
        }
        printf("\n");
        //printf("FROM HERE ON IS MAGNO\n");
        magnoSetup();
        //printf("New Heading: %d\n",getHeading());
        vTaskDelay(500);
    }
}

void ultrasonicTask(void *pvParameters)
{
    
    while (1)
    {
        ultraSetup();
        setDir(getCm(TRIG_PIN, ECHO_PIN));
        vTaskDelay(500);
    }
}

void vLaunch( void) {

    TaskHandle_t motorTaskHandle;
    xTaskCreate(motorTask, "motorThread", configMINIMAL_STACK_SIZE, NULL, 8, &motorTaskHandle);

    TaskHandle_t ultrasonicTaskHandle;
    xTaskCreate(ultrasonicTask, "ultrasonicThread", configMINIMAL_STACK_SIZE, NULL, 8, &ultrasonicTaskHandle);

    xControlQueue = xQueueCreate(10, sizeof(char));


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
