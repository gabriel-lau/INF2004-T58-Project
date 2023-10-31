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

const int INPUT_1_RIGHT = 19;
const int INPUT_2_RIGHT = 18;
const int PWM_RIGHT = 17;

uint16_t PWM_LEFT_CYCLE = 32768;
uint16_t PWM_RIGHT_CYCLE = 32768;
//uint16_t PWM_LEFT_CYCLE = 65535;
//uint16_t PWM_RIGHT_CYCLE = 65535;

int direction = -1;


void setDirection(int dir)
{
    direction = dir;
}


void moveForward()
{
    pwm_set_gpio_level(PWM_LEFT, PWM_LEFT_CYCLE);
    pwm_set_gpio_level(PWM_RIGHT, PWM_RIGHT_CYCLE);
    gpio_put(INPUT_1_LEFT, 1);
    gpio_put(INPUT_2_LEFT, 0);
    gpio_put(INPUT_1_RIGHT, 1);
    gpio_put(INPUT_2_RIGHT, 0);
}

void moveBackward()
{
    pwm_set_gpio_level(PWM_LEFT, PWM_LEFT_CYCLE);
    pwm_set_gpio_level(PWM_RIGHT, PWM_RIGHT_CYCLE);
    gpio_put(INPUT_1_LEFT, 0);
    gpio_put(INPUT_2_LEFT, 1);
    gpio_put(INPUT_1_RIGHT, 0);
    gpio_put(INPUT_2_RIGHT, 1);
}

void turnLeft()
{
    pwm_set_gpio_level(PWM_LEFT, PWM_LEFT_CYCLE);
    pwm_set_gpio_level(PWM_RIGHT, PWM_RIGHT_CYCLE);
    gpio_put(INPUT_1_LEFT, 0);
    gpio_put(INPUT_2_LEFT, 1);
    gpio_put(INPUT_1_RIGHT, 1);
    gpio_put(INPUT_2_RIGHT, 0);
}

void turnRight()
{
    pwm_set_gpio_level(PWM_LEFT, PWM_LEFT_CYCLE);
    pwm_set_gpio_level(PWM_RIGHT, PWM_RIGHT_CYCLE);
    gpio_put(INPUT_1_LEFT, 1);
    gpio_put(INPUT_2_LEFT, 0);
    gpio_put(INPUT_1_RIGHT, 0);
    gpio_put(INPUT_2_RIGHT, 1);
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

int main()
{
    stdio_init_all();

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

    while(1){
        if (direction == 1)
        {
            moveForward();
        }
        else if (direction == -1)
        {
            moveBackward();
        }
        else if (direction == 2)
        {
            turnLeft();
        }
        else if (direction == 3)
        {
            turnRight();
        }
        else if (direction == 0)
        {
            stop();
        }
        else 
        {
            direction = -1;
            if (PWM_LEFT_CYCLE == 65535)
            {
                PWM_LEFT_CYCLE = 32768;
                PWM_RIGHT_CYCLE = 32768;
            }
            else
            {
                PWM_LEFT_CYCLE = 65535;
                PWM_RIGHT_CYCLE = 65535;
            }
        }
        sleep_ms(5 * 1000);
        direction++;
    };

    return 0;
}
