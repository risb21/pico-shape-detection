#include <stdio.h>
#include "MPU6050.h"
#include "hardware/gpio.h"

// Arbirtary, the ANN has this many input nodes per acceleration recording
#define MAX_RECORD_LENGTH 83

bool record;
uint8_t current_recording;

// 83 data points, for 3 axes
float recorded_data[MAX_RECORD_LENGTH][3];

// Pins numbers
enum Pin {
    // buttons
    b_send = 13u,
    b_record,

    // LEDs
    
};


void recording(uint gpio, uint32_t events) {
    switch (events) {
    
    case GPIO_IRQ_EDGE_RISE:
        printf("Button pressed down\n");
        for (int i = 0; i < MAX_RECORD_LENGTH; i++)
            recorded_data[i][0] = recorded_data[i][0] = 
            recorded_data[i][0] = .0f;
        current_recording = 0;
        record = true;
        break;
    
    case GPIO_IRQ_EDGE_FALL:
        printf("Button released\n");
        record = false;
        printf("+-------+-------+-------+\n"
               "| acc_x | acc_y | acc_z |\n"
               "+-------+-------+-------+\n");
        for (int row = 10; row < MAX_RECORD_LENGTH; row++) {
            printf("|%7.3f|%7.3f|%7.3f|\n", recorded_data[row][0], recorded_data[row][1], recorded_data[row][2]);
            // sleep_ms(50);
        }
        printf("+-------+-------+-------+\n\n");
        break;

    default:
        printf("Invalid mode of triggering interrupt on pin: %d, event no.: %d", gpio, events);
        break;
    }
}

int main() {

    // setup phase
    stdio_init_all();
    // init MPU6050 library and wake
    MPU6050 mpu(16, 17);

    // irq setup
    gpio_init(Pin::b_record);
    gpio_set_dir(Pin::b_record, GPIO_IN);
    gpio_pull_down(Pin::b_record);
    gpio_set_irq_enabled_with_callback(Pin::b_record, GPIO_IRQ_EDGE_RISE, true, &recording);
    gpio_set_irq_enabled_with_callback(Pin::b_record, GPIO_IRQ_EDGE_FALL, true, &recording);

    record = false;
    current_recording = 0;
    for (int i = 0; i < MAX_RECORD_LENGTH; i++)
        recorded_data[i][0] = recorded_data[i][0] = 
        recorded_data[i][0] = .0f;

    // Job loop
    while (1) {
        if (current_recording < 83) {
            if (record) {
                acc_3D<float> read = mpu.read_acceleration();
                recorded_data[current_recording][0] = read.x;
                recorded_data[current_recording][1] = read.y;
                recorded_data[current_recording][2] = read.z;
                printf("Accelecration: x: %6.3f y: %6.3f z: %6.3f\n",
                        recorded_data[current_recording][0],
                        recorded_data[current_recording][1],
                        recorded_data[current_recording][2]
                );
                current_recording++;
            }
        } else {
            printf("Overflow!!!\n");
        }

        sleep_ms(50);
    }
}