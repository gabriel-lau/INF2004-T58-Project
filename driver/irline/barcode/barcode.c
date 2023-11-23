#include <string.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "barcode.h"

#include "FreeRTOS.h"
#include "task.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "hardware/clocks.h"

// int main()
// {
//   stdio_init_all();
//   barcode_init();

//   while(1){

//   }
// }

void resetReadingParameters()
{
  barcodeFlags.isPrevBlackBar = false;
  barcodeFlags.isBarcode = false;
  barcodeFlags.count = 0;
  barcodeFlags.limitter = 0;
  barType = BLACK_BAR;
  last_button_press_time = 0;
  coded_barcode = 0;
  decoded_barcode = 0;
  bar_index = 0;
  gpio_set_irq_enabled_with_callback(WALL_SENSOR_PIN, GPIO_IRQ_EDGE_RISE, true, &handleSensorInterrupt); // enable rising edge interrupt
}

void  initialiseBarcodeReader()
{
  printf("initializing barcode reader\n");

  // ADC Setup
  adc_init();
  adc_gpio_init(BARCODE_SENSOR_PIN);
  adc_select_input(BARCODE_ADC_CHANNEL);

  resetReadingParameters();

  gpio_set_irq_enabled_with_callback(WALL_SENSOR_PIN, GPIO_IRQ_EDGE_RISE, true, &handleSensorInterrupt); // enable rising edge interrupt
}

int captureInfraredSensorReading() {
  return adc_read();
}

void handleSensorInterrupt()
{
  if (time_us_64() - last_button_press_time > DEBOUNCE_DELAY_MS * 1000)
  {
    barcodeFlags.count++;
    last_button_press_time = time_us_64(); // update last button press time

    if (barcodeFlags.count > 1) // When wall is detected
    {
      // Disable interrupt and set flag
      gpio_set_irq_enabled_with_callback(WALL_SENSOR_PIN, GPIO_IRQ_EDGE_RISE, false, &handleSensorInterrupt); // enable rising edge interrupt
      barcodeFlags.isBarcode = true;

      printf("Barcode Detected please reverse robot\n");
      // TODO: Tell main to stop motors and reverse
      startBarcodeReadTask();
    }
  }
}

void startBarcodeReadTask() {
  xTaskCreate(readScannedBarcode, "BarcodeReadTask", 2048, NULL, 1, NULL);
} 
// void init_read_barcode()
// {
//   xTaskCreate(read_barcode, "read_barcode", 2048, NULL, 1, NULL);
// }

void decodeScannedBarcode(int black_bar_times[], int white_bar_times[])
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

  // printf("Black bar times: %d %d %d %d %d\n", dec_black_bar_times[0], dec_black_bar_times[1], dec_black_bar_times[2], dec_black_bar_times[3], dec_black_bar_times[4]);
  // printf("White bar times: %d %d %d %d\n\n", dec_white_bar_times[0], dec_white_bar_times[1], dec_white_bar_times[2], dec_white_bar_times[3]);

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

  // Set the two highest to 1, the rest to 0 for black bars
  for (int i = 0; i < 5; i++)
  {
    if (dec_black_bar_times[i] == max1 || dec_black_bar_times[i] == max2)
    {
      dec_black_bar_times[i] = 1;
    }
    else
    {
      dec_black_bar_times[i] = 0;
    }
  }

  // Find the highest value in white_bar_times
  int max_white = dec_white_bar_times[0];
  for (int i = 1; i < 4; i++)
  {
    if (dec_white_bar_times[i] > max_white)
    {
      max_white = dec_white_bar_times[i];
    }
  }

  // Set the highest to 1, the rest to 0 for white bars
  for (int i = 0; i < 4; i++)
  {
    if (dec_white_bar_times[i] == max_white)
    {
      dec_white_bar_times[i] = 1;
    }
    else
    {
      dec_white_bar_times[i] = 0;
    }
  }
  barcodeFlags.limitter++;
  printf("\nthis is the %d time\n", barcodeFlags.limitter);
  printf("Black bar times: %d %d %d %d %d\n", dec_black_bar_times[0], dec_black_bar_times[1], dec_black_bar_times[2], dec_black_bar_times[3], dec_black_bar_times[4]);
  printf("White bar times: %d %d %d %d\n\n", dec_white_bar_times[0], dec_white_bar_times[1], dec_white_bar_times[2], dec_white_bar_times[3]);
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

void readScannedBarcode()
{
  vTaskDelay(pdMS_TO_TICKS(1000));
  while (barcodeFlags.isBarcode)
  {
    uint16_t reading = captureInfraredSensorReading();

    if (reading > BARCODE_THRESHOLD && !barcodeFlags.isPrevBlackBar)
    {
      barcodeFlags.isPrevBlackBar = true;
      int timing = time_us_64() - last_button_press_time;
      black_bar_times[bar_index] = timing;
    }
    else if (reading < BARCODE_THRESHOLD && barcodeFlags.isPrevBlackBar)
    {
      barcodeFlags.isPrevBlackBar = false;
      int timing = time_us_64() - last_button_press_time;
      white_bar_times[bar_index] = timing;
      bar_index++;
    }
    if (white_bar_times[4] != 0)
    {
      decodeScannedBarcode(black_bar_times, white_bar_times);
      barcodeFlags.isPrevBlackBar = false;
      white_bar_times[4] = 0;
      bar_index = 0;
    }
    if (barcodeFlags.limitter > BARCODE_CHAR_LIMIT)
    {
      resetReadingParameters();
      vTaskDelete(NULL);
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}