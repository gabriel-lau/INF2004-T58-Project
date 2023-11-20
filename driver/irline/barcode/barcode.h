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

void barcode_init();
int get_ir_reading();
void check_if_wall();
void init_read_barcode();
void read_barcode();
void reset_barcode_params();
void barcode_to_char();

enum BarType
{
    BLACK_BAR,
    WHITE_BAR
};

struct Flags
{
    bool isPrevBlackBar;
    bool isBarcode;
    int count;
    int limitter;
};

static struct Flags barcodeFlags;
static enum BarType barType;
static uint64_t last_button_press_time = 0;
static uint16_t coded_barcode = 0;
static uint64_t decoded_barcode = 0;

static int black_bar_times[] = {0, 0, 0, 0, 0}; // Array for black bar times
static int white_bar_times[] = {0, 0, 0, 0, 0};    // Array for white bar times
static int bar_index = 0;                          // Index for the current bar

static char code_39_characters[] = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ-. *"; // Array of characters for code 39


#endif // BARCODE_H