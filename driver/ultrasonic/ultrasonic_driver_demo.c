// Get readings from ultrasonic sensor

#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/gpio.h"
#include "hardware/timer.h"

// Define GPIO 
const int TRIG_PIN = 0;
const int ECHO_PIN = 1;
int timeout = 26100;

// Store pulse timing information
uint64_t pulseLength = 0;
absolute_time_t startTime;
absolute_time_t endTime;

// Callback for GPIO interrupt events
void gpio_distance_callback(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_RISE) {
        // Rising edge event
        startTime = get_absolute_time();
    } else if (events & GPIO_IRQ_EDGE_FALL) {
        // Falling edge event
        endTime = get_absolute_time();
        pulseLength = absolute_time_diff_us(startTime, endTime);
        int distance = pulseLength / 29 / 2;  // Convert pulse length to distance in centimeters
        printf("Distance: %d cm\n", distance);
    }
}

// Initialize ultrasonic sensor pins
void setupUltrasonicPins(uint trigPin, uint echoPin) {
    gpio_init(trigPin);
    gpio_init(echoPin);
    gpio_set_dir(trigPin, GPIO_OUT);
    gpio_set_dir(echoPin, GPIO_IN);
}

// Send ultrasonic pulse
void getPulse(uint trigPin, uint echoPin) {
    gpio_put(trigPin, 1);
    sleep_us(10);
    gpio_put(trigPin, 0);

    // Enable GPIO interrupt on echo pin for both rising and falling edges
    gpio_set_irq_enabled_with_callback(echoPin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_distance_callback);
}

// Get distance in centimeters
void getCm(uint trigPin, uint echoPin) {
    getPulse(trigPin, echoPin);
}

// Main
int main() {
    stdio_init_all();
    
    while (1) {
        // Initialize sensor pins
        setupUltrasonicPins(TRIG_PIN, ECHO_PIN);
        
        // Trigger ultrasonic pulse and measure distance
        getCm(TRIG_PIN, ECHO_PIN);
        
        // Wait for 1 second before the next measurement
        sleep_ms(1000);
    }
}
