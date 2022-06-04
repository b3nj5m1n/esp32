#include "wifi.h"

static EventGroupHandle_t wifi_event_group;

static int s_retry_num = 0;

static const char *TAG = "WIFI";

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "Connecting to AP...");
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT &&
               event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < MAX_FAILURES) {
            ESP_LOGI(TAG, "Reconnecting to AP...");
            esp_wifi_connect();
            s_retry_num++;
        } else {
            xEventGroupSetBits(wifi_event_group, WIFI_FAILURE);
        }
    }
}

static void ip_event_handler(void *arg, esp_event_base_t event_base,
                             int32_t event_id, void *event_data) {
    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "STA IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_SUCCESS);
    }
}

esp_err_t connect_wifi(connect_to_server_config cfg) {
    int status = WIFI_FAILURE;

    // init esp netowrk interface
    ESP_ERROR_CHECK(esp_netif_init());

    // init default esp event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // create wifi station in wifi driver
    esp_netif_create_default_wifi_sta();

    // setup wifi station with default wifi config
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&config));

    wifi_event_group = xEventGroupCreate();

    esp_event_handler_instance_t wifi_handler_event_instance;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL,
        &wifi_handler_event_instance));

    esp_event_handler_instance_t got_ip_event_instance;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, NULL,
        &got_ip_event_instance));

    wifi_config_t wifi_config = {
        .sta = {.threshold.authmode = WIFI_AUTH_WPA2_PSK,
                .pmf_cfg = {.capable = true, .required = false}}};
    memcpy(wifi_config.sta.ssid, cfg.ap_ssid, 32);
    memcpy(wifi_config.sta.password, cfg.ap_password, 64);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "STA init complete");

    EventBits_t bits =
        xEventGroupWaitBits(wifi_event_group, WIFI_SUCCESS | WIFI_FAILURE,
                            pdFALSE, pdFALSE, portMAX_DELAY);

    if (bits & WIFI_SUCCESS) {
        ESP_LOGI(TAG, "Connected to AP");
        status = WIFI_SUCCESS;
    } else if (bits & WIFI_FAILURE) {
        ESP_LOGI(TAG, "Failed to connect to AP");
        status = WIFI_FAILURE;
    } else {
        ESP_LOGI(TAG, "Dunno what happened mate");
        status = WIFI_FAILURE;
    }

    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(
        IP_EVENT, IP_EVENT_STA_GOT_IP, got_ip_event_instance));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(
        WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_handler_event_instance));
    vEventGroupDelete(wifi_event_group);

    return status;
}

// returns socket
int connect_tcp_server(connect_to_server_config cfg) {
    struct sockaddr_in serverInfo = {0};

    serverInfo.sin_family = AF_INET;
    struct in_addr addr;
    inet_aton(cfg.server_ip, &addr);
    serverInfo.sin_addr.s_addr = addr.s_addr;
    serverInfo.sin_port = htons(cfg.server_port);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        ESP_LOGE(TAG, "Failed to create a socket..?");
        return TCP_FAILURE;
    }

    if (connect(sock, (struct sockaddr *)&serverInfo, sizeof(serverInfo)) !=
        0) {
        ESP_LOGE(TAG, "Failed to connect to %s!",
                 inet_ntoa(serverInfo.sin_addr.s_addr));
        close(sock);
        return TCP_FAILURE;
    }

    ESP_LOGI(TAG, "Connected to TCP server.");

    return sock;
}

// returns socket with tcp connection to server
int connect_to_server(connect_to_server_config cfg) {
    esp_err_t status = WIFI_FAILURE;

    status = connect_wifi(cfg);
    if (WIFI_SUCCESS != status) {
        ESP_LOGI(TAG, "Failed to associate to AP.");
        return -1;
    }

    return connect_tcp_server(cfg);
}
