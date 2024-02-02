#include "MPU6050.h"

MPU6050::MPU6050(int sda_pin = 4, int scl_pin = 5) {
    // 400kHz baud rate
    i2c_init(i2c_default, 400 * 1000);

    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);

    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(sda_pin, scl_pin, GPIO_FUNC_I2C));

    reset();
}

MPU6050::~MPU6050() {
}

// Read acceleration data into float values
acc_3D<float> MPU6050::read_acceleration() {
    
    uint8_t acc_buffer[6];

    // Register to query acceleration data
    uint8_t reg = 0x3B;
    i2c_write_blocking(i2c_default, address, &reg, 1, true); // true to keep master control of bus
    // Read into buffer as {x: HO, x: LO, y: HO, y: LO, z: HO, z: LO}
    i2c_read_blocking(i2c_default, address, acc_buffer, 6, false);

    return {
        .x = ((int16_t) (acc_buffer[0] << 8 | acc_buffer[1])) / acc_conversion_const,
        .y = ((int16_t) (acc_buffer[2] << 8 | acc_buffer[3])) / acc_conversion_const,
        .z = ((int16_t) (acc_buffer[4] << 8 | acc_buffer[5])) / acc_conversion_const
    };
}

// Non-functional for now
acc_3D<uint16_t> MPU6050::get_acc_offset() {
    
    uint8_t buffer[2];
    
    acc_3D<uint16_t> result;

    // x-axis acceleration offset
    uint8_t reg = 0x77;
    i2c_write_blocking(i2c_default, address, &reg, 1, true); // true to keep master control of bus
    i2c_read_blocking(i2c_default, address, buffer, 2, false);

    result.x = buffer[0] << 7 | buffer[0] >> 1;
    reg += 3;

    i2c_write_blocking(i2c_default, address, &reg, 1, true); // true to keep master control of bus
    i2c_read_blocking(i2c_default, address, buffer, 2, false);

    result.y = buffer[0] << 7 | buffer[0] >> 1;
    reg += 3;

    i2c_write_blocking(i2c_default, address, &reg, 1, true); // true to keep master control of bus
    i2c_read_blocking(i2c_default, address, buffer, 2, false);

    result.z = buffer[0] << 7 | buffer[0] >> 1;

    return result;

}

// Reset the MPU6050 and then wake it up
void MPU6050::reset() {
    // Two byte reset. First byte register, second byte data
    uint8_t reset[] = {0x6B, 0x80};
    i2c_write_blocking(i2c_default, address, reset, 2, false);
    sleep_ms(200);

    // Wake up mpu6050 from sleep mode (caused by reset)
    uint8_t wake[] = {0x6B, 0x00 | temp_disable};
    i2c_write_blocking(i2c_default, address, wake, 2, false);
    sleep_ms(200);
}