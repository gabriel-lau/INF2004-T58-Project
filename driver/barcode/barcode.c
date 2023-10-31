#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "pico/time.h"

// Constants
#define BARCODE_SENSOR_PIN 26
#define BARCODE_ADC_CHANNEL 0
#define BARCODE_THRESHOLD 3000

// Global Variables
static int black_bar_times[5] = {0, 0, 0, 0, 0};
static int white_bar_times[5] = {0, 0, 0, 0, 0};
static int bar_index = 0;
static bool isPrevBlackBar = false;
static uint64_t last_button_press_time = 0;

// Code 39 Characters and Patterns
const char CODE39_CHARS[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%*";
const char *CODE39_PATTERNS[] = {
    "NNWWN",  // 0
    "WNNNW",  // 1
    "NWNNW",  // 2
    "WWNNN",  // 3
    "NNWNW",  // 4
    "WNWNN",  // 5
    "NWWNN",  // 6
    "NNNWW",  // 7
    "WNNWN",  // 8
    "NWNWN",  // 9
    "WNNNW",  // A
    "NNWNNWNNW",  // B
    "WNWNNWNNN",  // C
    "NNNNWWNNW",  // D
    "WNNNWWNNN",  // E
    "NNWNWWNNN",  // F
    "NNNNNWWNW",  // G
    "WNNNNWWNN",  // H
    "NNWNNWWNN",  // I
    "NNNNWWWNN",  // J
    "WNNNNNNWW",  // K
    "NNWNNNNWW",  // L
    "WNWNNNNWN",  // M
    "NNNNWNNWW",  // N
    "WNNNWNNWN",  // O
    "NNWNWNNWN",  // P
    "NNNNNNWWW",  // Q
    "WNNNNNWWN",  // R
    "NNWNNNWWN",  // S
    "NNNNWNWWN",  // T
    "WWNNNNNNW",  // U
    "NWWNNNNNW",  // V
    "WWWNNNNNN",  // W
    "NWNNWNNNW",  // X
    "WWNNWNNNN",  // Y
    "NWWNWNNNN",  // Z
    "NWNNNNWNW",  // -
    "WWNNNNWNN",  // .
    "NWWNNNWNN",  // SPACE
    "NWNWNWNNN",  // $
    "NWNWNNNWN",  // /
    "NWNNNWNWN",  // +
    "NNNWNWNWN",  // %
    "NNWWN",  // *
};

// Function to get IR reading
int get_ir_reading() {
    uint16_t reading = adc_read();
    return reading;
}

// Function to decode barcode pattern
char decodeBarcodePattern(int black_bar_times[], int white_bar_times[]) {
    char pattern[6];
    int total_time = 0;
    for (int i = 0; i < 5; i++) {
        total_time += black_bar_times[i] + white_bar_times[i];
    }
    int avg_time = total_time / 10;  // Average time for one bar

    for (int i = 0; i < 5; i++) {
        pattern[i] = (black_bar_times[i] > avg_time) ? 'W' : 'N';
    }
    pattern[5] = '\0';

    printf("Trying to decode pattern: %s\n", pattern);  // Debug print

    for (int i = 0; i < sizeof(CODE39_CHARS) - 1; i++) {
        if (strcmp(pattern, CODE39_PATTERNS[i]) == 0) {
            return CODE39_CHARS[i];
        }
    }
    return '?';  // Unknown pattern
}


// Function to read barcode
void readBarcode() {
    uint16_t reading = get_ir_reading();

     printf("ADC Reading: %d\n", reading);  // Print ADC reading

    if (reading > BARCODE_THRESHOLD && !isPrevBlackBar) {
        isPrevBlackBar = true;
        int timing = time_us_64() - last_button_press_time;
        black_bar_times[bar_index] = timing;
    } else if (reading < BARCODE_THRESHOLD && isPrevBlackBar) {
        isPrevBlackBar = false;
        int timing = time_us_64() - last_button_press_time;
        white_bar_times[bar_index] = timing;
        bar_index++; 
    }

    if (white_bar_times[4] != 0) {
        printf("Black bar times: ");
        for (int i = 0; i < 5; i++) {
            printf("%d ", black_bar_times[i]);
        }
        printf("\n");

        printf("White bar times: ");
        for (int i = 0; i < 5; i++) {
            printf("%d ", white_bar_times[i]);
        }
        printf("\n");

        char decodedChar = decodeBarcodePattern(black_bar_times, white_bar_times);
        printf("Decoded character: %c\n", decodedChar);

        // Reset variables for next read
        isPrevBlackBar = false;
        white_bar_times[4] = 0;
        bar_index = 0;
    }
}

int main(void) {
    stdio_init_all();
    adc_init();
    adc_gpio_init(BARCODE_SENSOR_PIN);
    adc_select_input(BARCODE_ADC_CHANNEL);

    while (1) {
        readBarcode();
        sleep_ms(500);
    }
}