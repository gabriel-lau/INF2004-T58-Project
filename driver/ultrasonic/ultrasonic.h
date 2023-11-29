// GPIO pins for ULTRASONIC
extern int TRIG_PIN;
extern int ECHO_PIN;

int timeout = 26100;

absolute_time_t startTime;
absolute_time_t endTime;
uint64_t pulseLength;

int newDistance = 0;

// Get readings from ultrasonic sensor
uint64_t getPulse(uint trigPin, uint echoPin)
{
    gpio_put(trigPin, 1);
    sleep_us(10);
    gpio_put(trigPin, 0);

    uint64_t width = 0;

    while (gpio_get(echoPin) == 0) tight_loop_contents();
    absolute_time_t startTime = get_absolute_time();
    while (gpio_get(echoPin) == 1) 
    {
        width++;
        sleep_us(1);
        if (width > timeout) return 0;
    }
    absolute_time_t endTime = get_absolute_time();
    
    return absolute_time_diff_us(startTime, endTime);
}

// Get distance in cm
uint64_t getCm(uint trigPin, uint echoPin)
{
    uint64_t pulseLength = getPulse(trigPin, echoPin);
    return pulseLength / 29 / 2;
}

// Setup GPIO pins for ULTRASONIC
void setupUltrasonicPins()
{
    gpio_init(TRIG_PIN);
    gpio_init(ECHO_PIN);
    gpio_set_dir(TRIG_PIN, GPIO_OUT);
    gpio_set_dir(ECHO_PIN, GPIO_IN);
}

// Setup ultrasonic sensor
void ultraSetup() {
        setupUltrasonicPins(TRIG_PIN, ECHO_PIN);
        getCm(TRIG_PIN, ECHO_PIN);
}