#include "esp_event.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include <string.h>

#include "driver/gpio.h"
#include "nvs_flash.h"

#include "wifi.h"

#include "../../CONFIG.h"

#define LED_PIN 0

void handle(int socket) {
    char readBuffer[1024] = {0};
    char *msg = malloc(sizeof(readBuffer));

    bool stayin_alive = true;
    while (/* Ah, ha, ha, ha, */ stayin_alive) {
        bzero(readBuffer, sizeof(readBuffer));
        int r = read(socket, readBuffer, sizeof(readBuffer) - 1);
        for (int i = 0; i < r; i++) {
            putchar(readBuffer[i]);
        }

        bzero(msg, sizeof(readBuffer));
        memcpy(msg, readBuffer, r - 1);

        if (strcmp(msg, "ON") == 0) {
            gpio_set_level(LED_PIN, 1);
        } else if (strcmp(msg, "OFF") == 0) {
            gpio_set_level(LED_PIN, 0);
        } else if (strcmp(msg, "BYE") == 0) {
            stayin_alive = false;
            esp_deep_sleep_start();
        }
    }
}

void app_main(void) {
    // Init storage
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Init LED
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_PIN, 0);

    connect_to_server_config cfg = {
        .ap_ssid = WIFI_SSID,
        .ap_password = WIFI_PASSWORD,
        .server_ip = SERVER_IP,
        .server_port = SERVER_PORT,
    };
    int socket = connect_to_server(cfg);
    if (socket != -1) {
        handle(socket);
    } else {
        ESP_LOGE("main", "Something went wrong.");
        ESP_LOGI("main", "Restarting in 10 seconds.");
        vTaskDelay((10 * 1000) / portTICK_PERIOD_MS);
    }
}
