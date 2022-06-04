#include "esp_wifi.h"
#include "esp_log.h"

#include "freertos/event_groups.h"

#include "lwip/dns.h"
#include "lwip/err.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

#include <arpa/inet.h>

#define WIFI_SUCCESS 1 << 0
#define WIFI_FAILURE 1 << 1
#define TCP_SUCCESS 1 << 0
#define TCP_FAILURE -1
#define MAX_FAILURES 10

typedef struct {
    uint8_t ap_ssid[32];
    uint8_t ap_password[64];
    char *server_ip;
    int server_port;
} connect_to_server_config ;

int connect_to_server(connect_to_server_config cfg);
