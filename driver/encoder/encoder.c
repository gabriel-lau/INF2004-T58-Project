// PARTS USED
// 1. 1x Raspberry Pi Pico
// 2. 1x L298N Motor Driver
// 3. 2x DC Motors
// 4. 2x Wheels
// 5. 2x Wheel Encoders
// 5. 1x 6V Battery

#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/pwm.h"

#define PI 3.14159

// GPIO pins for MOTOR
const int INPUT_1_LEFT = 12;
const int INPUT_2_LEFT = 13;
const int PWM_LEFT = 14;
const int ENCODER_LEFT = 15;

const int INPUT_1_RIGHT = 19;
const int INPUT_2_RIGHT = 18;
const int PWM_RIGHT = 17;
const int ENCODER_RIGHT = 16;

uint16_t PWM_LEFT_CYCLE = 32768;
uint16_t PWM_RIGHT_CYCLE = 32768;

// Wheel dimensions
const double WHEEL_DIAMETER = 6.5;
const double WHEEL_CIRCUMFERENCE = WHEEL_DIAMETER * PI;
const double PULSES_PER_REVOLUTION = 40;
const double DISTANCE_PER_PULSE = WHEEL_CIRCUMFERENCE / PULSES_PER_REVOLUTION;

// Encoder
double leftDistanceTravelled = 0;
double rightDistanceTravelled = 0;
double targetDisance;

// PID constants
// PID gains 
double kp = 2.0;
double ki = 0.1; 
double kd = 1.0;

double prevError = 0;
double integral = 0; 

void moveForward()
{
    pwm_set_gpio_level(PWM_LEFT, PWM_LEFT_CYCLE);
    pwm_set_gpio_level(PWM_RIGHT, PWM_RIGHT_CYCLE);
    gpio_put(INPUT_1_LEFT, 1);
    gpio_put(INPUT_2_LEFT, 0);
    gpio_put(INPUT_1_RIGHT, 1);
    gpio_put(INPUT_2_RIGHT, 0);
}

void stop()
{
    pwm_set_gpio_level(PWM_LEFT, PWM_LEFT_CYCLE);
    pwm_set_gpio_level(PWM_RIGHT, PWM_RIGHT_CYCLE);
    gpio_put(INPUT_1_LEFT, 0);
    gpio_put(INPUT_2_LEFT, 0);
    gpio_put(INPUT_1_RIGHT, 0);
    gpio_put(INPUT_2_RIGHT, 0);
}

void controlLoop() {

    double *slowMotor;
    double *fastMotor;

    // Read current position
    if (leftDistanceTravelled == rightDistanceTravelled)
    {
        return;
    }
    else if (leftDistanceTravelled > rightDistanceTravelled)
    {
        // Adjust right motor
        slowMotor = &rightDistanceTravelled;
        fastMotor = &leftDistanceTravelled;
    }
    else
    {
        // Adjust left motor
        slowMotor = &leftDistanceTravelled;
        fastMotor = &rightDistanceTravelled;
    }

    //double currentPosition = readPosition(); // meters

    // Calculate error
    double error = *slowMotor - *fastMotor;

    // Integral term
    integral += error * DISTANCE_PER_PULSE * 2;

    // Derivative term
    double derivative = (error - prevError) / DISTANCE_PER_PULSE * 2;

    // Calculate output
    double output = kp * error + ki * integral + kd * derivative;

    // Update motor
    //updateMotor(output); // * 180
    if (slowMotor == &leftDistanceTravelled)
    {
        PWM_LEFT_CYCLE = PWM_LEFT_CYCLE + output * 180;
    }
    else
    {
        PWM_RIGHT_CYCLE = PWM_RIGHT_CYCLE + output * 180;
    }

    // Update previous error
    prevError = error;
}


void gpio_encoder_changed_callback(uint gpio, uint32_t events) {
    static double leftLastTime;
    static double rightLastTime;
    if (events & GPIO_IRQ_EDGE_RISE )
    {
        if (gpio == ENCODER_LEFT)
        {
            leftLastTime = time_us_32();
        }
        else if (gpio == ENCODER_RIGHT)
        {
            rightLastTime = time_us_32();
        }
    }
    else
    {
        if (gpio == ENCODER_LEFT)
        {
            double currTime = time_us_32();
            //double timeDiff = currTime - leftLastTime;
            leftDistanceTravelled += DISTANCE_PER_PULSE * 2;
            printf("L Distance Travelled: %f\n", leftDistanceTravelled);
            // leftCurrentSpeed = DISTANCE_PER_PULSE / (timeDiff / 1000000.0);
            // printf("Wheel Speed: %f\n", leftCurrentSpeed);
            leftLastTime = currTime;
        }
        else if (gpio == ENCODER_RIGHT)
        {
            double currTime = time_us_32();
            //double timeDiff = currTime - rightLastTime;
            //printf("Time Diff: %f\n", timeDiff);
            rightDistanceTravelled += DISTANCE_PER_PULSE * 2;
            printf("R Distance Travelled: %f\n", rightDistanceTravelled);
            //rightCurrentSpeed = DISTANCE_PER_PULSE / (timeDiff / 1000000.0);
            // printf("Wheel Speed: %f\n", rightCurrentSpeed);
            rightLastTime = currTime;
        }
    }
}


int main()
{
    stdio_init_all();

    //Init Left GPIO
    gpio_init(INPUT_1_LEFT);
    gpio_init(INPUT_2_LEFT);
    gpio_init(PWM_LEFT);
    gpio_init(ENCODER_LEFT);

    //Init Right GPIO
    gpio_init(INPUT_1_RIGHT);
    gpio_init(INPUT_2_RIGHT);
    gpio_init(PWM_RIGHT);
    gpio_init(ENCODER_RIGHT);

    //Set Left GPIO to out power board
    gpio_set_dir(INPUT_1_LEFT, GPIO_OUT);
    gpio_set_dir(INPUT_2_LEFT, GPIO_OUT);
    gpio_set_function(PWM_LEFT, GPIO_FUNC_PWM);
    gpio_set_dir(ENCODER_LEFT, GPIO_IN);

    //Set Right GPIO to out power board
    gpio_set_dir(INPUT_1_RIGHT, GPIO_OUT);
    gpio_set_dir(INPUT_2_RIGHT, GPIO_OUT);
    gpio_set_function(PWM_RIGHT, GPIO_FUNC_PWM);
    gpio_set_dir(ENCODER_RIGHT, GPIO_IN);

    // Find out which PWM slice is connected to GPIO
    uint slice_num_left = pwm_gpio_to_slice_num(PWM_LEFT);
    uint slice_num_right = pwm_gpio_to_slice_num(PWM_RIGHT);

    pwm_set_gpio_level(PWM_LEFT, PWM_LEFT_CYCLE);
    pwm_set_gpio_level(PWM_RIGHT, PWM_RIGHT_CYCLE);

    pwm_set_enabled(slice_num_left, true);
    pwm_set_enabled(slice_num_right, true);

    gpio_set_irq_enabled_with_callback(ENCODER_LEFT, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_encoder_changed_callback);
    gpio_set_irq_enabled_with_callback(ENCODER_RIGHT, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_encoder_changed_callback);
    while(1){
        moveForward();
        controlLoop();
        //sleep_ms(5 * 1000);
        //stop();
        //sleep_ms(5 * 1000);
    };

    return 0;
}
