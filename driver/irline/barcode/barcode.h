#ifndef BARCODE_H
#define BARCODE_H

// every barcode char is 9 bars
// start w black end with black
// 5 black bar (+0-9)
// 4 white bar (+0,+10,+20,+30)
// narrow = 0
// thick = 1

// Define GPIO pin and ADC channel for the "front" IR sensor
#define BARCODE_SENSOR_PIN 26
#define WALL_SENSOR_PIN 22
#define BARCODE_ADC_CHANNEL 0

#define DEBOUNCE_DELAY_MS 100

#define BARCODE_THRESHOLD 1500
#define BARCODE_CHAR_LIMIT 2

void initialiseBarcodeReader();
int  captureInfraredSensorReading();
void handleSensorInterrupt();
void startBarcodeReadTask();
void readScannedBarcode();
void resetReadingParameters();
void convertBarcodeToCharacter();

enum currentBarType
{
    BLACK_BAR,
    WHITE_BAR
};

struct Flags
{
    bool isPreviousBlackBarDetected;
    bool isBarcodeDetected;
    int wallDetectionCount;
    int readingLimitReached;
};

static struct Flags barcodeFlags;
static enum currentBarType currentBarType;
static uint64_t timeOfLastDetectedBar = 0;
static uint16_t coded_barcode = 0;
static uint64_t decoded_barcode = 0;

static int black_bar_times[] = {0, 0, 0, 0, 0}; // Array for black bar times
static int white_bar_times[] = {0, 0, 0, 0, 0};    // Array for white bar times
static int currentBarIndex = 0;                         // Index for the current bar

static char code_39_characters[] = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ-. *"; // Array of characters for code 39


#endif // BARCODE_H