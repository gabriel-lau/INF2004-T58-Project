#ifndef BARCODE_H
#define BARCODE_H

// Configure the GPIO pin connected to the line tracking sensor
#define BARCODE_SENSOR_PIN 26
#define BARCODE_ADC_CHANNEL 0
#define BARCODE_BLACK_THRESHOLD 1200
#define BARCODE_WHITE_THRESHOLD 1000
#define BLACK_BAR_COUNT 5
#define WHITE_BAR_COUNT 4
#define MAX_PULSE_WIDTHS 9

// Define the states of the FSM
typedef enum {
    STATE_WAITING_FOR_START,  // Waiting for the start signal
    STATE_DETECTING_BLACK,    // Detecting a black bar
    STATE_DETECTING_WHITE,    // Detecting a white bar
} BarcodeState;

// Structure to represent the mapping
typedef struct {
    const char* pattern;
    char code39_character;
} BarcodeMapping;

// Define an array of barcode-to-Code 39 mappings
BarcodeMapping barcodeMappings[] = {
    { "000110100", '0' },
    { "100100001", '1' },
    { "001100001", '2' },
    { "101100000", '3' },
    { "000110001", '4' },
    { "100110000", '5' },
    { "001110000", '6' },
    { "000100101", '7' },
    { "100100100", '8' },
    { "001100100", '9' },
    { "100001001", 'A' },
    { "001001001", 'B' },
    { "101001000", 'C' },
    { "000011001", 'D' },
    { "100011000", 'E' },
    { "001011000", 'F' },
    { "000001101", 'G' },
    { "100001100", 'H' },
    { "001001100", 'I' },
    { "100000011", 'J' },
    { "001000011", 'K' },
    { "101000010", 'L' },
    { "000010011", 'M' },
    { "100010010", 'N' },
    { "001010010", 'O' },
    { "000000111", 'P' },
    { "100000110", 'Q' },
    { "001000110", 'R' },
    { "000010110", 'S' },
    { "100010100", 'T' },
    { "001010100", 'U' },
    { "100110001", 'V' },
    { "001110001", 'W' },
    { "000100111", 'X' },
    { "100100110", 'Y' },
    { "001100110", 'Z' },
    { "010010100", '*' },
    { "010001001", '-' },
    { "001001010", '.' },
    { "010101000", ' ' }
};

#endif
