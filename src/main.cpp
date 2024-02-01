#include <stdio.h>
#include "MPU6050.h"

int main() {

    stdio_init_all();
    MPU6050 mpu(16, 17);

    while (1) {
        auto [off_x, off_y, off_z] = mpu.get_acc_offset();
        auto [x, y, z] = mpu.read_acceleration();
        printf("Acceleration: x: %6.3f, y: %6.3f, z: %6.3f\n", x, y, z);
        printf("Offset by: x: %d, y: %d, z: %d\n", off_x, off_y, off_z);

        sleep_ms(100);
    }
}