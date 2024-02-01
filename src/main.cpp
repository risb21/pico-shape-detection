#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

int main() {
    stdio_init_all();

    uint8_t line = 0;

    while (1) {
        printf("Hi from custom project, line: %4d\r", line++);
        sleep_ms(500);
    }

}

