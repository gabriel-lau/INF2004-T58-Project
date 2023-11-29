// For tracking the lines on either side of the car
int irLine(int lineTrackingPin)
{

    if (gpio_get(lineTrackingPin) == 0)
    {
        // The line tracking sensor detected a line
        printf("No Black line detected\n");
    }
    else if (gpio_get(lineTrackingPin) == 1)
    {
        printf("Black line detected\n");
    }
    else
    {
        // The line tracking sensor didn't detect a line
        printf("Line detected\n");
    }

    // Add a delay to control the update rate
    sleep_ms(100); // Adjust the delay as needed

    return gpio_get(lineTrackingPin);
}
