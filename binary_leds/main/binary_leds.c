#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "sdkconfig.h"
#include <stdio.h>

#include "driver/gpio.h"

#define LED_PIN_1 22
#define LED_PIN_2 21
#define LED_PIN_3 19
#define LED_PIN_4 5
#define LED_PIN_5 16
#define LED_PIN_6 0
#define LED_PIN_7 2
#define LED_PIN_8 15

typedef unsigned char byte;

int pins[8] = {
    LED_PIN_1, LED_PIN_2, LED_PIN_3, LED_PIN_4,
    LED_PIN_5, LED_PIN_6, LED_PIN_7, LED_PIN_8,
};

void reset(int pin) {
    gpio_reset_pin(pin);
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
}

void init() {
    for (int i = 0; i < 8; i++) {
        reset(pins[i]);
    }
}

void light_bit(byte b, int index) {
    if ((b & (1 << (7 - index))) >> (7 - index)) {
        gpio_set_level(pins[index], 1);
    } else {
        gpio_set_level(pins[index], 0);
    }
}

void light_byte(byte b) {
    for (int i = 0; i < 8; i++) {
        light_bit(b, i);
    }
}

void light_string(char *s) {
    int i = 0;
    while (s[i] != '\0') {
        light_byte(s[i]);
        i++;
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        light_byte(0);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void ripple() {
    for (int j = 0; j < 3; j++) {
        for (int i = 0; i < 8; i++) {
            light_byte(1 << (7 - i));
            vTaskDelay(170 / portTICK_PERIOD_MS);
        }
    }
}

void app_main(void) {
    init();
    while (true) {
        light_string("Hello, world!");
        ripple();
    }
}
