#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <math.h> // Include the math library for atan2 and M_PI

#define LSM303DLHC_ADDRESS 0x19 // 7-bit I2C address of the LSM303DLHC accelerometer/magnetometer
#define MAG_ADDRESS 0x1E
int16_t x_avg = 0, y_avg = 0, z_avg = 0;
int readings = 0;

int newHeading = 0;

int getHeading(){
    return newHeading;
}

float get_heading(int16_t x_mag, int16_t y_mag)
{
    float heading = atan2((float)y_mag, (float)x_mag);
    heading = heading * (180.0 / M_PI);
    heading += 0.05;
    if (heading < 0)
        heading += 360.0;
    return heading;
}

void lsm303dlhc_init(i2c_inst_t *i2c)
{
    // Initialize the I2C interface
    i2c_init(i2c, 100000);
    gpio_set_function(0, GPIO_FUNC_I2C);
    gpio_set_function(1, GPIO_FUNC_I2C);
    i2c_set_slave_mode(i2c, false, 0);
    gpio_set_pulls(0, true, true); // Set pull-ups on GPIO 0
    gpio_set_pulls(1, true, true); // Set pull-ups on GPIO 1

    // Initialize the LSM303DLHC accelerometer
    uint8_t data[2];
    data[0] = 0x20; // CTRL_REG1_A register
    data[1] = 0x27; // Enable the accelerometer (50 Hz, normal mode)
    i2c_write_blocking(i2c, LSM303DLHC_ADDRESS, data, 2, false);

    // Initialize the LSM303DLHC magnetometer
    data[0] = 0x02; // CRA_REG_M register
    data[1] = 0x00; // Set the data rate to the default (0.75 Hz)
    i2c_write_blocking(i2c, MAG_ADDRESS, data, 2, false);
}
void lsm303dlhc_read_acceleration(i2c_inst_t *i2c, int16_t *x, int16_t *y, int16_t *z)
{
    uint8_t data[6];
    data[0] = 0xA8 | 0x80; // OUT_X_L_A address with MSB set for auto-increment
    i2c_write_blocking(i2c, LSM303DLHC_ADDRESS, data, 1, true);
    i2c_read_blocking(i2c, LSM303DLHC_ADDRESS, data, 6, false);

    // Extract acceleration values
    *x = (int16_t)((data[1] << 8) | data[0]);
    *y = (int16_t)((data[3] << 8) | data[2]);
    *z = (int16_t)((data[5] << 8) | data[4]);
}
void lsm303dlhc_read_magnetometer(i2c_inst_t *i2c, int16_t *x, int16_t *y, int16_t *z)
{
    uint8_t data[6];
    data[0] = 0x03; // OUT_X_H_M address
    i2c_write_blocking(i2c, MAG_ADDRESS, data, 1, true);
    i2c_read_blocking(i2c, MAG_ADDRESS, data, 6, false);

    // Extract magnetometer values
    *x = (int16_t)((data[0] << 8) | data[1]);
    *z = (int16_t)((data[2] << 8) | data[3]);
    *y = (int16_t)((data[4] << 8) | data[5]);
}

int magnoSetup()
{
    i2c_inst_t *i2c = i2c0;

    lsm303dlhc_init(i2c);

        int16_t x_acc, y_acc, z_acc;
        lsm303dlhc_read_acceleration(i2c, &x_acc, &y_acc, &z_acc);
        // printf("Acceleration - X: %d, Y: %d, Z: %d\n", x_acc, y_acc, z_acc);

        int16_t x_mag, y_mag, z_mag;
        lsm303dlhc_read_magnetometer(i2c, &x_mag, &y_mag, &z_mag);
        // printf("Magnetometer - X: %d, Y: %d, Z: %d\n", x_mag, y_mag, z_mag);

        float heading = get_heading(x_mag, y_mag);
        printf("Heading: %.2f degrees\n", heading);
        newHeading = heading;
        // sleep_ms(100);
    

    return 0;
}
