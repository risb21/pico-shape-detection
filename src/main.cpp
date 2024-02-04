#include <stdio.h>

#include "MPU6050.hpp"
#include "tflite_wrapper.hpp"
#include "shape_model.hpp"

#include "hardware/gpio.h"
#include "pico/cyw43_arch.h"

// Arbirtary, the ANN has this many input nodes per axis (2-D input)
#define MAX_RECORD_LEN 83

uint8_t curr_row, flags;

// 83 data points, for 3 axes
acc_3D<float> *rec_data;

// Pin numbers for each function,
// set according to wiring
enum Pin {
    // onboard LED
    cyw43_led = 0u,

    // buttons
    b_predict = 13u,
    b_record,

    // I2C pins
    mpu_sda = 16u,
    mpu_scl,

    // LEDs

};

// Flags for operations
enum Flag {
    record = 0b1,
    print_once = 0b1 << 1,
    predict = 0b1 << 2,
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
    printf("    +---------+---------+---------+\n"
           "    |  acc_x  |  acc_y  |  acc_z  |\n"
           "    +---------+---------+---------+\n");
    for (int row = 0; row < 5; row++)
        printf("%2d. | %7.3f | %7.3f | %7.3f |\n", row+1,
                rec_data[row].x,
                rec_data[row].y,
                rec_data[row].z
        );

    printf("    +---------+---------+---------+\n"
           "                  ...              \n"
           "    +---------+---------+---------+\n");
    for (int row = MAX_RECORD_LEN - 5; row < MAX_RECORD_LEN; row++)
        printf("%2d. | %7.3f | %7.3f | %7.3f |\n", row+1,
                rec_data[row].x,
                rec_data[row].y,
                rec_data[row].z
        );

    printf("    +---------+---------+---------+\n");
}


void handle_irq(uint gpio, uint32_t events) {
    // Handle predict button
    if (gpio == Pin::b_predict) {
        flags |= Flag::predict;
        printf("Predicting...\n");
        return;
    }

    // Handle Record button
    switch (events) {
    
    case GPIO_IRQ_EDGE_RISE:
        printf("Button pressed down\n");

        struct_memset<float>(rec_data, .0f, MAX_RECORD_LEN);
        curr_row = 0;
        // Set record flag
        flags |= Flag::record;

        cyw43_arch_gpio_put(Pin::cyw43_led, 1);
        break;
    
    case GPIO_IRQ_EDGE_FALL:
        printf("Button released\n");
        
        // unset record flag
        flags &= 0xFF ^ Flag::record;
        // Set print once flag
        flags |= Flag::print_once;

        cyw43_arch_gpio_put(Pin::cyw43_led, 0);
        break;

    default:
        printf("Invalid mode of triggering interrupt on pin: %d, event no.: %d\n"
               "Restarting...\n", gpio, events);
        handle_irq(gpio, GPIO_IRQ_EDGE_RISE);
    }
}

int main() {

 /* -------------
     setup phase 
    ------------- */
    stdio_init_all();
    sleep_ms(500);

    if (cyw43_arch_init()) {
        printf("[ERROR] Wi-Fi init failed\n");
        return -1;
    }
    cyw43_arch_gpio_get(Pin::cyw43_led);

    printf("[OK] cyw43 wireless module initialized\n");

    // init MPU6050 library and wake
    MPU6050 mpu(Pin::mpu_sda, Pin::mpu_scl);
    printf("[OK] MPU6050 accelerometer initialized\n");

    // IRQ setup
    // Record button
    gpio_init(Pin::b_record);
    gpio_set_dir(Pin::b_record, GPIO_IN);
    gpio_pull_down(Pin::b_record);
    gpio_set_irq_enabled_with_callback(Pin::b_record, GPIO_IRQ_EDGE_RISE, true, &handle_irq);
    gpio_set_irq_enabled_with_callback(Pin::b_record, GPIO_IRQ_EDGE_FALL, true, &handle_irq);
    // Predict button
    gpio_init(Pin::b_predict);
    gpio_set_dir(Pin::b_predict, GPIO_IN);
    gpio_pull_down(Pin::b_predict);
    gpio_set_irq_enabled_with_callback(Pin::b_predict, GPIO_IRQ_EDGE_RISE, true, &handle_irq);

    flags = 0b0;
    curr_row = 0;
    rec_data = new acc_3D<float>[MAX_RECORD_LEN];
    struct_memset<float>(rec_data, .0f, MAX_RECORD_LEN);

    // Initialize ML model
    TFLMicro model(shape_model, shape_model_len);
    int init_status = model.init();

    if (!init_status) {
        printf("[ERROR] Could not initialize model\n");
        return -1;
    }

    printf("[OK] Machine learning model initialized\n");

    printf("[OK] Device Ready\n");

 /* -----------
     Job loop 
    ----------- */
    while (true) {

        if (flags & Flag::print_once) {
            // Unset print once flag
            flags &= 0xFF ^ Flag::print_once;
            print_data();
        }

        if (flags & Flag::record) {
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

        if (flags & Flag::predict) {
            // Unset predict flag
            flags &= 0xFF ^ Flag::predict;

            model.input_data(rec_data, MAX_RECORD_LEN);
            float *predictions = reinterpret_cast<float *>(model.predict());

            if (predictions == nullptr) {
                printf("Error in predicting shape\n");
                continue;
            }

            printf("+----------+----------+----------+\n"
                   "|  Circle  |  Square  | Triangle |\n"
                   "+----------+----------+----------+\n"
                   "| %8.4f | %8.4f | %8.4f |\n"
                   "+----------+----------+----------+\n",
                   predictions[0], predictions[1], predictions[2]);
        }

        sleep_ms(20);
    }
}