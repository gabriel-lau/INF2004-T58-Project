// GPIO pins for MOTOR
extern int INPUT_1_LEFT;
extern int INPUT_2_LEFT;
extern int PWM_LEFT;
extern int INPUT_1_RIGHT;
extern int INPUT_2_RIGHT;
extern int PWM_RIGHT;
extern uint16_t PWM_LEFT_CYCLE;
extern uint16_t PWM_RIGHT_CYCLE;

double leftDutyCycle = 18750;
double rightDutyCycle = 18125; 
int leftAngleTime = 2000;
int rightAngleTime = 2300;

void setSpeed()
{
    uint left_slice_num = pwm_gpio_to_slice_num(PWM_LEFT);
    uint right_slice_num = pwm_gpio_to_slice_num(PWM_RIGHT);

    pwm_set_clkdiv(left_slice_num, 100); // 125MHz / 100 = 1.25MHz
    pwm_set_wrap(left_slice_num, 62500); // 1.25MHz / 62500 = 20Hz

    pwm_set_chan_level(left_slice_num, PWM_CHAN_A, leftDutyCycle);
    pwm_set_chan_level(left_slice_num, PWM_CHAN_B, leftDutyCycle);

    pwm_set_clkdiv(right_slice_num, 100);
    pwm_set_wrap(right_slice_num, 62500);

    pwm_set_chan_level(right_slice_num, PWM_CHAN_A, rightDutyCycle);
    pwm_set_chan_level(right_slice_num, PWM_CHAN_B, rightDutyCycle);

    pwm_set_enabled(left_slice_num, true);
    pwm_set_enabled(right_slice_num, true);
}

// code to move forward
void moveForward()
{
    // pwm_set_gpio_level(PWM_LEFT, PWM_LEFT_CYCLE);
    // pwm_set_gpio_level(PWM_RIGHT, PWM_RIGHT_CYCLE);
    gpio_put(INPUT_1_LEFT, 1);
    gpio_put(INPUT_2_LEFT, 0);
    gpio_put(INPUT_1_RIGHT, 1);
    gpio_put(INPUT_2_RIGHT, 0);
    setSpeed();
}

// code to move backwards
void moveBackward()
{
    // pwm_set_gpio_level(PWM_LEFT, PWM_LEFT_CYCLE);
    // pwm_set_gpio_level(PWM_RIGHT, PWM_RIGHT_CYCLE);
    gpio_put(INPUT_1_LEFT, 0);
    gpio_put(INPUT_2_LEFT, 1);
    gpio_put(INPUT_1_RIGHT, 0);
    gpio_put(INPUT_2_RIGHT, 1);
    setSpeed();
}

// code to turn left
void turnLeft()
{
    // pwm_set_gpio_level(PWM_LEFT, PWM_LEFT_CYCLE);
    // pwm_set_gpio_level(PWM_RIGHT, PWM_RIGHT_CYCLE);
    gpio_put(INPUT_1_LEFT, 0);
    gpio_put(INPUT_2_LEFT, 1);
    gpio_put(INPUT_1_RIGHT, 1);
    gpio_put(INPUT_2_RIGHT, 0);
    setSpeed();
}

// code to turn right
void turnRight()
{
    // pwm_set_gpio_level(PWM_LEFT, PWM_LEFT_CYCLE);
    // pwm_set_gpio_level(PWM_RIGHT, PWM_RIGHT_CYCLE);
    gpio_put(INPUT_1_LEFT, 1);
    gpio_put(INPUT_2_LEFT, 0);
    gpio_put(INPUT_1_RIGHT, 0);
    gpio_put(INPUT_2_RIGHT, 1);
    setSpeed();
}

// code to stop moving
void stop()
{
    // pwm_set_gpio_level(PWM_LEFT, PWM_LEFT_CYCLE);
    // pwm_set_gpio_level(PWM_RIGHT, PWM_RIGHT_CYCLE);
    gpio_put(INPUT_1_LEFT, 0);
    gpio_put(INPUT_2_LEFT, 0);
    gpio_put(INPUT_1_RIGHT, 0);
    gpio_put(INPUT_2_RIGHT, 0);
}
