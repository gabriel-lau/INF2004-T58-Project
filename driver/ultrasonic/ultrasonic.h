//Get readings from ultrasonic sensor

#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/gpio.h"
#include "hardware/timer.h"

const int TRIG_PIN = 2;
const int ECHO_PIN = 3;

int timeout = 26100;

uint64_t pulseLength = 0; 

absolute_time_t startTime;
absolute_time_t endTime;
uint64_t pulseLength;

int newDistance = 0;

int getNewDistance() {
    return newDistance;
}

void gpio_distance_callback(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_RISE)
    {
        startTime = get_absolute_time();
        //printf("Start time: %lld\n", startTime);
    }
    else if (events & GPIO_IRQ_EDGE_FALL)
    {
        printf("Start time: %lld\n", startTime);
        endTime = get_absolute_time();
        printf("End time: %lld\n", endTime);
        pulseLength = absolute_time_diff_us(startTime, endTime);
        int distance = pulseLength / 29 / 2;
        printf("Distance: %d cm\n", distance);
        newDistance = distance;
    }
}

void setupUltrasonicPins(uint trigPin, uint echoPin)
{
    gpio_init(trigPin);
    gpio_init(echoPin);
    gpio_set_dir(trigPin, GPIO_OUT);
    gpio_set_dir(echoPin, GPIO_IN);
}

void getPulse(uint trigPin, uint echoPin)
{
    gpio_put(trigPin, 1);
    sleep_us(10);
    gpio_put(trigPin, 0);

    //uint64_t width = 0;

    gpio_set_irq_enabled_with_callback(echoPin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_distance_callback);
    
}

void getCm(uint trigPin, uint echoPin)
{
    getPulse(trigPin, echoPin);
    //return pulseLength / 29 / 2;
}

void ultraSetup() {

        setupUltrasonicPins(TRIG_PIN, ECHO_PIN);
        getCm(TRIG_PIN, ECHO_PIN);
        sleep_ms(1000);
    
}