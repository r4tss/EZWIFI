#include <string.h>
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mesh.h"
#include "nvs_flash.h"

#define SSID "CDIO_MESH"
#define PASS "CDIO_PASS"

#define MESH_AP_AUTHMODE WIFI_AUTH_WPA2_PSK
#define MESH_AP_CONNECTIONS 5
#define MESH_NON_MESH_AP_CONNECTIONS 2
#define MESH_AP_PASS "CDIO_PASS"

#define MESH_CHANNEL 0
#define MESH_ROUTER_SSID "CDIO_MESH"
#define MESH_ROUTER_PASS "CDIO_PASS"

static const char *TAG = "wifi_main";
static const uint8_t MESH_ID[6] = { 0x77, 0x77, 0x77, 0x77, 0x77, 0x77};
static mesh_addr_t mesh_parent_addr;
static int mesh_layer = -1;
static esp_netif_t *netif_sta = NULL;

void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

void setup_station(void);

void setup_ap(void);

void mesh_scan_done_handler(int num);

void mesh_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

void ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

void ezmesh_init(void);
