#include <string.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"

#include "FreeRTOS.h"
#include "task.h"

#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "hardware/clocks.h"

#include "barcode.h"

volatile bool isBlackBar = false;
static uint64_t lastBarTime = 0;

BarcodeState currentState = STATE_WAITING_FOR_START;
static int blackBarCount = 0;
static int whiteBarCount = 0;
static int done_black = 0;
static int done_white = 0;

#define MAX_PULSE_WIDTHS 9                      // Adjust this to your desired array size
static int pulseWidthCount = 0;                 // Counter for the number of stored pulseWidth values
static uint64_t pulseWidths[MAX_PULSE_WIDTHS];  // Array to store pulseWidth values

int get_ir_reading()
{
  uint16_t reading = adc_read();
  return reading;
}

// Function to find the Code 39 character for a given barcode pattern
char findCode39Character(const char* pattern) {
    int numMappings = sizeof(barcodeMappings) / sizeof(barcodeMappings[0]);
    for (int i = 0; i < numMappings; i++) {
        if (strcmp(barcodeMappings[i].pattern, pattern) == 0) {
            return barcodeMappings[i].code39_character;
        }
    }
    return '?'; // Character not found
}

void detect_barcode() {
  int reading = get_ir_reading();
  int currentTime = time_us_64();
  int pulseWidth = currentTime - lastBarTime;

  switch (currentState) {
    case STATE_WAITING_FOR_START:
      if (!isBlackBar && reading > BARCODE_BLACK_THRESHOLD) {
        // Detected the start signal (black bar)
        currentState = STATE_DETECTING_WHITE;
        isBlackBar = true;
        lastBarTime = currentTime;
        blackBarCount = 1;
      }
    break;

    case STATE_DETECTING_BLACK:
      if (!isBlackBar && reading > BARCODE_BLACK_THRESHOLD) {
        // Detected another black bar
        currentState = STATE_DETECTING_WHITE;
        isBlackBar = true;
        lastBarTime = currentTime;
        blackBarCount++;
        pulseWidths[pulseWidthCount++] = pulseWidth; // Store pulseWidth
        printf("Detected White Bar - Pulse Width: %d\n", pulseWidth);
        if (blackBarCount > BLACK_BAR_COUNT) {
          printf("Barcode Overload!\n");
        }
      }
    break;

    case STATE_DETECTING_WHITE:
      if (isBlackBar && reading < BARCODE_WHITE_THRESHOLD) {
        // Detected a white bar
        isBlackBar = false;
        lastBarTime = currentTime;
        whiteBarCount++;
        pulseWidths[pulseWidthCount++] = pulseWidth; // Store pulseWidth
        printf("Detected Black Bar - Pulse Width: %d\n", pulseWidth);
        if (whiteBarCount > WHITE_BAR_COUNT) {
          // Successfully detected the Code 39 pattern
          printf("Code 39 Pattern Detected!\n");
          // Calculate the average pulse width
          uint64_t averagePulseWidth = 0;
          for (int i = 0; i < pulseWidthCount; i++) {
              averagePulseWidth += pulseWidths[i];
          }
          averagePulseWidth /= pulseWidthCount;

          // Set a threshold to differentiate narrow and wide bars
          uint64_t threshold = averagePulseWidth;

          // Initialize an array to store narrow (0) or wide (1) bar classifications
          int barcodeClassification[pulseWidthCount];

          for (int i = 0; i < pulseWidthCount; i++) {
              if (pulseWidths[i] > threshold) {
                  barcodeClassification[i] = 1;  // Wide bar
              } else {
                  barcodeClassification[i] = 0;  // Narrow bar
              }
          }

          char barcodePattern[MAX_PULSE_WIDTHS] = ""; // Initialize an empty string
          char str[1];

          for (int i = 0; i < pulseWidthCount; i++) {
            sprintf(str, "%d", barcodeClassification[i]);
            strcat(barcodePattern, str);
          }

          char code39Character = findCode39Character(barcodePattern);
          printf("Barcode Pattern: %s => Code 39 Character: %c\n", barcodePattern, code39Character);


          // Reset the FSM to wait for the next barcode
          blackBarCount = 0;
          whiteBarCount = 0;
          pulseWidthCount = 0;
          currentState = STATE_WAITING_FOR_START;
        } else {
          currentState = STATE_DETECTING_BLACK;
        }
      }
      break;
  }
}


void read_barcode(__unused void *params) {
    gpio_init(BARCODE_SENSOR_PIN);
    gpio_set_dir(BARCODE_SENSOR_PIN, GPIO_IN);

    adc_init();
    adc_gpio_init(BARCODE_SENSOR_PIN);    
    adc_select_input(BARCODE_ADC_CHANNEL);  

    while(true) {
      detect_barcode();
    }
}
void vLaunch(void) {
  TaskHandle_t barcode_task;
  xTaskCreate(read_barcode, "BarcodeThread", configMINIMAL_STACK_SIZE, NULL, 1, &barcode_task);

#if NO_SYS && configUSE_CORE_AFFINITY && configNUM_CORES > 1
  // we must bind the main task to one core (well at least while the init is called)
  // (note we only do this in NO_SYS mode, because cyw43_arch_freertos
  // takes care of it otherwise)
  vTaskCoreAffinitySet(task, 1);
#endif

  /* Start the tasks and timer running. */
  vTaskStartScheduler();
}

int main(void) {
    // Initialize the Pico SDK
    stdio_init_all();

    /* Configure the hardware ready to run the demo. */
    const char *rtos_name;
    #if ( portSUPPORT_SMP == 1 )
        rtos_name = "FreeRTOS SMP";
    #else
        rtos_name = "FreeRTOS";
    #endif

    #if ( portSUPPORT_SMP == 1 ) && ( configNUM_CORES == 2 )
        printf("Starting %s on both cores:\n", rtos_name);
        vLaunch();
    #elif ( RUN_FREERTOS_ON_CORE == 1 )
        printf("Starting %s on core 1:\n", rtos_name);
        multicore_launch_core1(vLaunch);
        while (true);
    #else
        printf("Starting %s on core 0:\n", rtos_name);
        vLaunch();
    #endif
        return 0;
}