#include <stdio.h>

#include "MPU6050.hpp"
#include "tflite_wrapper.hpp"

#include "hardware/gpio.h"
#include "pico/cyw43_arch.h"

// Arbirtary, the ANN has this many input nodes per acceleration recording
#define MAX_RECORD_LEN 83

bool record, print_once;
uint8_t curr_row;

// 83 data points, for 3 axes
acc_3D<float> *rec_data;

// Pin numbers for each function,
// set according to wiring
enum Pin {
    // onboard LED
    cyw43_led = 0u,

    // buttons
    b_send = 13u,
    b_record,

    // I2C pins
    mpu_sda = 16u,
    mpu_scl,

    // LEDs

};

template<typename T>
void struct_memset(acc_3D<T> *acc, T val, size_t size) {
    for (int i = 0; i < size; i++)
        acc[i] = {
            .x = val,
            .y = val,
            .z = val
        };
}

void print_data() {
    printf("    +-------+-------+-------+\n"
           "    | acc_x | acc_y | acc_z |\n"
           "    +-------+-------+-------+\n");
    for (int row = 0; row < 5; row++)
        printf("%2d. |%7.3f|%7.3f|%7.3f|\n", row+1,
                rec_data[row].x,
                rec_data[row].y,
                rec_data[row].z
        );

    printf("    +-------+-------+-------+\n"
           "               ...           \n"
           "    +-------+-------+-------+\n");
    for (int row = MAX_RECORD_LEN - 5; row < MAX_RECORD_LEN; row++)
        printf("%2d. |%7.3f|%7.3f|%7.3f|\n", row+1,
                rec_data[row].x,
                rec_data[row].y,
                rec_data[row].z
        );

    printf("    +-------+-------+-------+\n");
}


void recording(uint gpio, uint32_t events) {

    switch (events) {
    
    case GPIO_IRQ_EDGE_RISE:
        printf("Button pressed down\n");

        struct_memset<float>(rec_data, .0f, MAX_RECORD_LEN);
        curr_row = 0;
        record = true;
        cyw43_arch_gpio_put(Pin::cyw43_led, 1);
        break;
    
    case GPIO_IRQ_EDGE_FALL:
        printf("Button released\n");
        
        record = false;
        print_once = true;
        cyw43_arch_gpio_put(Pin::cyw43_led, 0);
        break;

    default:
        printf("Invalid mode of triggering interrupt on pin: %d, event no.: %d\n"
               "Restarting...\n", gpio, events);
        recording(gpio, GPIO_IRQ_EDGE_RISE);
    }
}

int main() {

    // setup phase
    stdio_init_all();
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return -1;
    }

    printf("LED state: %d\n", cyw43_arch_gpio_get(Pin::cyw43_led));

    // init MPU6050 library and wake
    MPU6050 mpu(Pin::mpu_sda, Pin::mpu_scl);

    // irq setup
    gpio_init(Pin::b_record);
    gpio_set_dir(Pin::b_record, GPIO_IN);
    gpio_pull_down(Pin::b_record);
    gpio_set_irq_enabled_with_callback(Pin::b_record, GPIO_IRQ_EDGE_RISE, true, &recording);
    gpio_set_irq_enabled_with_callback(Pin::b_record, GPIO_IRQ_EDGE_FALL, true, &recording);

    record = print_once = false;
    curr_row = 0;
    rec_data = new acc_3D<float>[MAX_RECORD_LEN];
    struct_memset<float>(rec_data, .0f, MAX_RECORD_LEN);

    printf("[OK] Device Ready\n");

    // Job loop
    while (true) {
        if (rec_data == nullptr) {
            printf("Could not allocate struct array\r");
            sleep_ms(1000);
            continue;
        }

        if (print_once) {
            print_once = false;
            print_data();
        }

        if (record) {
            if (curr_row < 83) {
                rec_data[curr_row] = mpu.read_acceleration();

                // Print slow enough to read values
                if (curr_row % 5 == 0)
                    printf("Acceleration: x: %6.3f y: %6.3f z: %6.3f\r",
                            rec_data[curr_row].x,
                            rec_data[curr_row].y,
                            rec_data[curr_row].z
                    );
                curr_row++;
            } else {
                printf("Overflow!!!\n");
            }
        }

        sleep_ms(20);
    }
}