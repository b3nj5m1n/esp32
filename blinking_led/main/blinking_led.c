#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "sdkconfig.h"
#include <stdio.h>

#include "driver/gpio.h"

#define LED_PIN 0

int millisecond(int millisecond) { return (millisecond / portTICK_PERIOD_MS); }

void sleep_seconds(int seconds) { vTaskDelay(millisecond(seconds * 1000)); }

void app_main(void) {
    char *task_name = pcTaskGetName(NULL);
    ESP_LOGI(task_name, "Light it up\n");

    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    while (1) {
        gpio_set_level(LED_PIN, 1);
        sleep_seconds(1);
        gpio_set_level(LED_PIN, 0);
        sleep_seconds(1);
    }
}
