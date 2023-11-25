#include <string.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"

#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "hardware/clocks.h"

// Resets all parameters related to barcode reading to their initial state.
// Resets all parameters related to barcode reading to their initial state.
void resetReadingParameters()
{
  // Reset flags and counters
  barcodeFlags.isPreviousBlackBarDetected = false;
  barcodeFlags.isBarcodeDetected = false;
  barcodeFlags.wallDetectionCount = 0;
  barcodeFlags.readingLimitReached = 0;
  // Reset bar type and timings
  currentBarType = BLACK_BAR;
  timeOfLastDetectedBar = 0;
  // Reset barcode values and index
  // Reset flags and counters
  barcodeFlags.isPreviousBlackBarDetected = false;
  barcodeFlags.isBarcodeDetected = false;
  barcodeFlags.wallDetectionCount = 0;
  barcodeFlags.readingLimitReached = 0;
  // Reset bar type and timings
  currentBarType = BLACK_BAR;
  timeOfLastDetectedBar = 0;
  // Reset barcode values and index
  coded_barcode = 0;
  decoded_barcode = 0;
  currentBarIndex = 0;
  // Enable GPIO interrupt for detecting barcodes
  gpio_set_irq_enabled_with_callback(WALL_SENSOR_PIN, GPIO_IRQ_EDGE_RISE, true, &handleSensorInterrupt);
}

// Initializes the barcode reader by setting up the ADC and GPIO interrupts.
void initialiseBarcodeReader()
// Initializes the barcode reader by setting up the ADC and GPIO interrupts.
void initialiseBarcodeReader()
{
  printf("Initializing barcode reader\n");

  // Initialize ADC for reading barcode sensor data
  // Initialize ADC for reading barcode sensor data
  adc_init();
  adc_gpio_init(BARCODE_SENSOR_PIN);
  adc_select_input(BARCODE_ADC_CHANNEL);

  // Reset reading parameters to initial state
  resetReadingParameters();

  // Enable GPIO interrupt for wall sensor
  gpio_set_irq_enabled_with_callback(WALL_SENSOR_PIN, GPIO_IRQ_EDGE_RISE, true, &handleSensorInterrupt);
}

// Captures a single reading from the infrared sensor.
// Captures a single reading from the infrared sensor.
int captureInfraredSensorReading() {
  return adc_read();
}

// Handles the interrupt triggered by the wall sensor detecting a barcode.
// Handles the interrupt triggered by the wall sensor detecting a barcode.
void handleSensorInterrupt()
{
  // Check for debounce to avoid noise in the signal
  if (time_us_64() - timeOfLastDetectedBar > DEBOUNCE_DELAY_MS * 1000)
  {
    // Increment wall detection count and update timestamp
    barcodeFlags.wallDetectionCount++;
    timeOfLastDetectedBar = time_us_64();
    // Increment wall detection count and update timestamp
    barcodeFlags.wallDetectionCount++;
    timeOfLastDetectedBar = time_us_64();

    if (barcodeFlags.wallDetectionCount > 1) // When a wall (or barcode) is detected more than once
    if (barcodeFlags.wallDetectionCount > 1) // When a wall (or barcode) is detected more than once
    {
      // Disable further interrupts and set barcode detection flag
      gpio_set_irq_enabled(WALL_SENSOR_PIN, GPIO_IRQ_EDGE_RISE, false);
      barcodeFlags.isBarcodeDetected = true;
      // Disable further interrupts and set barcode detection flag
      gpio_set_irq_enabled(WALL_SENSOR_PIN, GPIO_IRQ_EDGE_RISE, false);
      barcodeFlags.isBarcodeDetected = true;

      // Notify user to reverse the robot and start the barcode reading task
      // Notify user to reverse the robot and start the barcode reading task
      printf("Barcode Detected please reverse robot\n");
      startBarcodeReadTask();
    }
  }
}
// Creates a new FreeRTOS task to handle barcode reading.
// Creates a new FreeRTOS task to handle barcode reading.
void startBarcodeReadTask() {
  xTaskCreate(readScannedBarcode, "BarcodeReadTask", 2048, NULL, 1, NULL);
}


char decode_barcode(int black_bar_times[], int white_bar_times[])
{
  int dec_black_bar_times[] = {0, 0, 0, 0, 0}; // Array for black bar times
  int dec_white_bar_times[] = {0, 0, 0, 0, 0}; // Array for white bar times
  dec_black_bar_times[0] = (white_bar_times[0] - black_bar_times[0]) / 10000;
  dec_black_bar_times[1] = (white_bar_times[1] - black_bar_times[1]) / 10000;
  dec_black_bar_times[2] = (white_bar_times[2] - black_bar_times[2]) / 10000;
  dec_black_bar_times[3] = (white_bar_times[3] - black_bar_times[3]) / 10000;
  dec_black_bar_times[4] = (white_bar_times[4] - black_bar_times[4]) / 10000;

  dec_white_bar_times[0] = (black_bar_times[1] - white_bar_times[0]) / 10000;
  dec_white_bar_times[1] = (black_bar_times[2] - white_bar_times[1]) / 10000;
  dec_white_bar_times[2] = (black_bar_times[3] - white_bar_times[2]) / 10000;
  dec_white_bar_times[3] = (black_bar_times[4] - white_bar_times[3]) / 10000;

  int max1 = 0;
  int max2 = 0;

  for (int i = 0; i < 5; i++)
  {
    if (dec_black_bar_times[i] > max1)
    {
      max2 = max1;
      max1 = dec_black_bar_times[i];
    }
    else if (dec_black_bar_times[i] > max2)
    {
      max2 = dec_black_bar_times[i];
    }
  }

// Decodes the barcode from the black and white bar times.
void decodeScannedBarcode(int black_bar_times[], int white_bar_times[])
{
  // Arrays to hold normalized bar times
  int dec_black_bar_times[5];
  int dec_white_bar_times[4];

  // Normalize the black bar times by subtracting the start of the black bar time from the start of the white bar time
  for (int i = 0; i < 5; i++)
  {
    dec_black_bar_times[i] = (white_bar_times[i] - black_bar_times[i]) / 10000;
    if (i < 4) // Avoid accessing white_bar_times[4] which does not exist
    {
      // Normalize the white bar times similarly
      dec_white_bar_times[i] = (black_bar_times[i + 1] - white_bar_times[i]) / 10000;
    }
  }

  // Variables to hold the maximum values for normalization
  int max1 = 0;
  int max2 = 0;

  // Find the two largest black bar times
  for (int i = 0; i < 5; i++)
  {
    if (dec_black_bar_times[i] > max1)
    {
      max2 = max1;
      max1 = dec_black_bar_times[i];
    }
    else if (dec_black_bar_times[i] > max2)
    {
      max2 = dec_black_bar_times[i];
    }
  }

  // Normalize black bars to binary values (1 or 0)
  for (int i = 0; i < 5; i++)
  {
    dec_black_bar_times[i] = (dec_black_bar_times[i] == max1 || dec_black_bar_times[i] == max2) ? 1 : 0;
  }

  // Find the largest white bar time
  int max_white = dec_white_bar_times[0];
  for (int i = 1; i < 4; i++)
  {
    if (dec_white_bar_times[i] > max_white)
    {
      max_white = dec_white_bar_times[i];
    }
  }

  // Normalize white bars to binary values (1 or 0)
  for (int i = 0; i < 4; i++)
  {
    dec_white_bar_times[i] = (dec_white_bar_times[i] == max_white) ? 1 : 0;
  }

  // Increment the number of times barcode reading has occurred
  barcodeFlags.readingLimitReached++;
  // Print the normalized bar times for debugging
  printf("\nthis is the %d time\n", barcodeFlags.readingLimitReached);
  printf("Black bar times: %d %d %d %d %d\n", dec_black_bar_times[0], dec_black_bar_times[1], dec_black_bar_times[2], dec_black_bar_times[3], dec_black_bar_times[4]);
  printf("White bar times: %d %d %d %d\n\n", dec_white_bar_times[0], dec_white_bar_times[1], dec_white_bar_times[2], dec_white_bar_times[3]);

  // Convert the barcode to a character using the normalized times
  convertBarcodeToCharacter(dec_black_bar_times, dec_white_bar_times);
}

void convertBarcodeToCharacter(int black_bar_times[], int white_bar_times[])
{
  printf("Decoding barcode");
  int result = 0;

  if (black_bar_times[0] && black_bar_times[4])
    result += 1;
  else if (black_bar_times[1] && black_bar_times[4])
    result += 2;
  else if (black_bar_times[0] && black_bar_times[1])
    result += 3;
  else if (black_bar_times[2] && black_bar_times[4])
    result += 4;
  else if (black_bar_times[0] && black_bar_times[2])
    result += 5;
  else if (black_bar_times[1] && black_bar_times[2])
    result += 6;
  else if (black_bar_times[3] && black_bar_times[4])
    result += 7;
  else if (black_bar_times[0] && black_bar_times[3])
    result += 8;
  else if (black_bar_times[1] && black_bar_times[3])
    result += 9;
  else if (black_bar_times[2] && black_bar_times[3])
    result += 10;

  if (white_bar_times[1])
    result += 0;
  else if (white_bar_times[2])
    result += 9;
  else if (white_bar_times[3])
    result += 19;
  else if (white_bar_times[0])
    result += 29;

  printf("Result: %d\n", result);
  char decoded_char = code_39_characters[result];
  printf("Decoded character: %c\n", decoded_char);
}

// Continuously reads the barcode sensor and processes the data to decode barcodes.
void readScannedBarcode()
{
  // Delay to stabilize the sensor readings
  vTaskDelay(pdMS_TO_TICKS(1000));

  // Loop as long as a barcode is detected
  while (barcodeFlags.isBarcodeDetected)
  {
    // Capture the sensor reading
    uint16_t reading = captureInfraredSensorReading();
    // Calculate the time since the last bar was detected
    uint64_t timing = time_us_64() - timeOfLastDetectedBar;

    // Check if a black bar has been detected
    if (reading > BARCODE_THRESHOLD && !barcodeFlags.isPreviousBlackBarDetected)
    {
      // Update flags and timings for the black bar
      barcodeFlags.isPreviousBlackBarDetected = true;
      black_bar_times[currentBarIndex] = timing;
    }
    else if (reading < BARCODE_THRESHOLD && barcodeFlags.isPreviousBlackBarDetected)
    {
      // Update flags and timings for the white space following the black bar
      barcodeFlags.isPreviousBlackBarDetected = false;
      white_bar_times[currentBarIndex] = timing;
      // Move to the next set of bars
      currentBarIndex++;
    }

    // Check if we have read the full set of bars for a single character
    if (white_bar_times[4] != 0)
    {
      // Decode the scanned barcode character
      decodeScannedBarcode(black_bar_times, white_bar_times);
      // Reset the flags and timings for the next character
      barcodeFlags.isPreviousBlackBarDetected = false;
      white_bar_times[4] = 0;
      currentBarIndex = 0;
    }

    // If we have reached the limit for reading characters, reset parameters and delete the task
    if (barcodeFlags.readingLimitReached > BARCODE_CHAR_LIMIT)
    {
      resetReadingParameters();
      vTaskDelete(NULL);
    }

    // Delay a short period before the next read
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}