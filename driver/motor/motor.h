#define PI 3.14159

// GPIO pins for MOTOR
extern int INPUT_1_LEFT;
extern int INPUT_2_LEFT;
extern int PWM_LEFT;
extern int INPUT_1_RIGHT;
extern int INPUT_2_RIGHT;
extern int PWM_RIGHT;
extern uint16_t PWM_LEFT_CYCLE;
extern uint16_t PWM_RIGHT_CYCLE;

// GPIO pins for ENCODER
extern int ENCODER_LEFT;
extern int ENCODER_RIGHT;

// Wheel dimensions
const double WHEEL_DIAMETER = 6.5;
const double WHEEL_CIRCUMFERENCE = WHEEL_DIAMETER * PI;
const double PULSES_PER_REVOLUTION = 40;
const double DISTANCE_PER_PULSE = WHEEL_CIRCUMFERENCE / PULSES_PER_REVOLUTION;

// Starting speed
float desiredSpeed = 15;
double leftSpeed = 0;
double rightSpeed = 0;

// Starting duty cycle
double leftDutyCycle = 18750; 
double rightDutyCycle = 17500;

// Turning time
int leftAngleTime = 2400;
int rightAngleTime = 2500;

// PID constants
// PID gains 
float Kp = 1.0; 
float Ki = 0.1; 
float Kd = 0.01; 

float leftPrevError = 0;
float leftIntegral = 0;
float rightPrevError = 0;
float rightIntegral = 0;

// Function to setup the GPIO pins for the motor
void motorSetup() 
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
}

// Function to compute the control signal
float controlLoop(float setpoint, float current_value, float *integral, float *prev_error) {
    float error =  setpoint - current_value;
    *integral += error;
    float derivative = error - *prev_error;
    float control_signal = Kp * error + Ki * (*integral) + Kd * derivative;
    *prev_error = error;
    return control_signal;
}

// Function to setup the GPIO pins for the encoder
void gpio_encoder_changed_callback(uint gpio, uint32_t events) {
    // Static variables to keep track of the last time the encoder was triggered
    static double leftLastTime;
    static double rightLastTime;

    // Get the time of the rising edge of the pulse
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
        // Calculate the speed of the wheel with the time difference between the rising and falling edge
        if (gpio == ENCODER_LEFT)
        {
            double currTime = time_us_32();
            double timeDiff = currTime - leftLastTime;
            double leftSpeed = DISTANCE_PER_PULSE / (timeDiff / 1000000);
            printf("L speed: %f\n", leftSpeed);

            // Calculate the control signal
            float diff = controlLoop(desiredSpeed, leftSpeed, &leftIntegral, &leftPrevError) * 10;
            // printf("L diff: %f\n", diff);
            leftDutyCycle += diff;
            leftLastTime = currTime;
        }
        else if (gpio == ENCODER_RIGHT)
        {
            double currTime = time_us_32();
            double timeDiff = currTime - rightLastTime;
            double rightSpeed = DISTANCE_PER_PULSE / (timeDiff / 1000000);
            printf("R speed: %f\n", rightSpeed);

            // Calculate the control signal
            float diff = controlLoop(desiredSpeed, rightSpeed, &rightIntegral, &rightPrevError) * 10;
            // printf("R diff: %f\n", diff);
            rightDutyCycle += diff;
            rightLastTime = currTime;
        }
    }
}

// Function to set the speed of the wheels
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

// Function to move the car forward
void moveForward()
{
    gpio_put(INPUT_1_LEFT, 1);
    gpio_put(INPUT_2_LEFT, 0);
    gpio_put(INPUT_1_RIGHT, 1);
    gpio_put(INPUT_2_RIGHT, 0);

    setSpeed();
    gpio_set_irq_enabled_with_callback(ENCODER_LEFT, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_encoder_changed_callback);
    gpio_set_irq_enabled_with_callback(ENCODER_RIGHT, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_encoder_changed_callback);
}

// Function to move the car backward
void moveBackward()
{
    setSpeed();
    gpio_put(INPUT_1_LEFT, 0);
    gpio_put(INPUT_2_LEFT, 1);
    gpio_put(INPUT_1_RIGHT, 0);
    gpio_put(INPUT_2_RIGHT, 1);
    gpio_set_irq_enabled_with_callback(ENCODER_LEFT, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_encoder_changed_callback);
    gpio_set_irq_enabled_with_callback(ENCODER_RIGHT, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_encoder_changed_callback);

}

// Function to turn the car left
void turnLeft()
{
    setSpeed();
    gpio_put(INPUT_1_LEFT, 0);
    gpio_put(INPUT_2_LEFT, 1);
    gpio_put(INPUT_1_RIGHT, 1);
    gpio_put(INPUT_2_RIGHT, 0);
    gpio_set_irq_enabled_with_callback(ENCODER_LEFT, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_encoder_changed_callback);
    gpio_set_irq_enabled_with_callback(ENCODER_RIGHT, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_encoder_changed_callback);
}

// Function to turn the car right
void turnRight()
{
    setSpeed();
    gpio_put(INPUT_1_LEFT, 1);
    gpio_put(INPUT_2_LEFT, 0);
    gpio_put(INPUT_1_RIGHT, 0);
    gpio_put(INPUT_2_RIGHT, 1);
    gpio_set_irq_enabled_with_callback(ENCODER_LEFT, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_encoder_changed_callback);
    gpio_set_irq_enabled_with_callback(ENCODER_RIGHT, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_encoder_changed_callback);
}

// Function to turn the car 90 degrees right
void turn90Right()
{
    setSpeed();
    gpio_put(INPUT_1_LEFT, 1);
    gpio_put(INPUT_2_LEFT, 0);
    gpio_put(INPUT_1_RIGHT, 0);
    gpio_put(INPUT_2_RIGHT, 1);
    sleep_ms(rightAngleTime);
    stop();
}

// Function to turn the car 90 degrees left
void turn90Left()
{
    setSpeed();
    gpio_put(INPUT_1_LEFT, 0);
    gpio_put(INPUT_2_LEFT, 1);
    gpio_put(INPUT_1_RIGHT, 1);
    gpio_put(INPUT_2_RIGHT, 0);
    sleep_ms(leftAngleTime);
    stop();
}

// Function to stop the car
void stop()
{
    setSpeed();
    gpio_put(INPUT_1_LEFT, 0);
    gpio_put(INPUT_2_LEFT, 0);
    gpio_put(INPUT_1_RIGHT, 0);
    gpio_put(INPUT_2_RIGHT, 0);
}
