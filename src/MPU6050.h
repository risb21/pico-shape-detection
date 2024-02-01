#ifndef MPU6050_H_
#define MPU6050_H_

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"

template <typename T>
struct acc_3D{
    T x, y, z;
};

class MPU6050 {

public:
    MPU6050(int sda_pin, int scl_pin);
    ~MPU6050();
    
    acc_3D<float> read_acceleration();
    acc_3D<uint16_t> get_acc_offset();

private:
    static const uint8_t address = 0x68;

    // flags for MPU6050 commands
    static constexpr uint8_t temp_disable = 1 << 3;
    // Multiplicative factor for acceleration, as per spec
    static constexpr float acc_conversion_const = 1 << 14;
    
    void reset();
};

#endif