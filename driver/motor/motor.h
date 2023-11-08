// GPIO pins for MOTOR
extern int INPUT_1_LEFT;
extern int INPUT_2_LEFT;
extern int PWM_LEFT;
extern int INPUT_1_RIGHT;
extern int INPUT_2_RIGHT;
extern int PWM_RIGHT;
extern uint16_t PWM_LEFT_CYCLE;
extern uint16_t PWM_RIGHT_CYCLE;

void moveForward()
{
    pwm_set_gpio_level(PWM_LEFT, PWM_LEFT_CYCLE);
    pwm_set_gpio_level(PWM_RIGHT, PWM_RIGHT_CYCLE);
    gpio_put(INPUT_1_LEFT, 1);
    gpio_put(INPUT_2_LEFT, 0);
    gpio_put(INPUT_1_RIGHT, 1);
    gpio_put(INPUT_2_RIGHT, 0);
}
