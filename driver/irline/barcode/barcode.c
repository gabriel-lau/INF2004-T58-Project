#include <string.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"

#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"
#include "queue.h"

#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "hardware/clocks.h"

#include "barcode.h"

void reset_barcode_params()
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
  gpio_set_irq_enabled_with_callback(WALL_SENSOR_PIN, GPIO_IRQ_EDGE_RISE, true, &check_if_wall); // enable rising edge interrupt
}

void barcode_init()
{
  printf("initializing barcode\n");
  // Initialize the ADC
  adc_init();
  adc_gpio_init(BARCODE_SENSOR_PIN);
  adc_select_input(BARCODE_ADC_CHANNEL);

  reset_barcode_params();

  gpio_set_irq_enabled_with_callback(WALL_SENSOR_PIN, GPIO_IRQ_EDGE_RISE, true, &check_if_wall); // enable rising edge interrupt
}

int get_ir_reading()
{
  uint16_t reading = adc_read();
  return reading;
}

void check_if_wall()
{
  if (time_us_64() - last_button_press_time > DEBOUNCE_DELAY_MS * 1000)
  {
    barcodeFlags.count++;
    last_button_press_time = time_us_64(); // update last button press time

    if (barcodeFlags.count > 1) // When wall is detected
    {
      // Disable interrupt and set flag
      gpio_set_irq_enabled_with_callback(WALL_SENSOR_PIN, GPIO_IRQ_EDGE_RISE, false, &check_if_wall); // enable rising edge interrupt
      barcodeFlags.isBarcode = true;

      printf("Barcode Detected please reverse robot\n");
      // TODO: Tell main to stop motors and reverse
      init_read_barcode();
    }
  }
}

void init_read_barcode()
{
  xTaskCreate(read_barcode, "BarcodeThread", configMINIMAL_STACK_SIZE, NULL, 4, &barcodeTaskHandle);
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
  return barcode_to_char(dec_black_bar_times, dec_white_bar_times);
}

char barcode_to_char(int black_bar_times[], int white_bar_times[])
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
  return decoded_char;
}

void read_barcode()
{
  vTaskDelay(pdMS_TO_TICKS(1000));
  while (barcodeFlags.isBarcode)
  {
    uint16_t reading = get_ir_reading();
    // printf("Reading: %d\n", reading);

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
      char barcode_char = decode_barcode(black_bar_times, white_bar_times);

      printf("%c", barcode_char);
      // Convert float to JSON string
      char barcodeBuffer[mbaTASK_MESSAGE_BUFFER_SIZE]; // Adjust the size based on your needs
      snprintf(barcodeBuffer, sizeof(barcodeBuffer), "{\"temp\": %c}", barcode_char);

      // Send JSON data to wifiTask
      xMessageBufferSend(
          xBarcodeMessageBuffer,     /* The message buffer to write to. */
          (void *)barcodeBuffer,     /* The source of the data to send. */
          strlen(barcodeBuffer) + 1, /* Include null-terminator in length */
          0);                        /* Do not block, should the buffer be full. */

      barcodeFlags.isPrevBlackBar = false;
      white_bar_times[4] = 0;
      bar_index = 0;
    }
    if (barcodeFlags.limitter > BARCODE_CHAR_LIMIT)
    {
      reset_barcode_params();
      vTaskDelete(barcodeTaskHandle);
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}