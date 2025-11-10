#include <string.h>
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mesh.h"
#include "nvs_flash.h"
#include <pthread.h>
#include <stdio.h>

#define SSID "CDIO"
#define PASS "CDIO_PASS"

#define WIFI_AP_AUTHMODE WIFI_AUTH_WPA2_PSK
#define WIFI_CHANNEL 0

#define MESH_AP_AUTHMODE WIFI_AUTH_WPA2_PSK
#define MESH_AP_CONNECTIONS 5
#define MESH_NON_MESH_AP_CONNECTIONS 2
#define MESH_AP_PASS "CDIO_PASS"

#define MESH_CHANNEL 0
#define MESH_ROUTER_SSID "CDIO_MESH"
#define MESH_ROUTER_PASS "CDIO_PASS"

#define MAX_STA_CONN 10

static const char *TAG = "wifi_main";
static const uint8_t MESH_ID[6] = { 0x77, 0x77, 0x77, 0x77, 0x77, 0x77};
static mesh_addr_t mesh_parent_addr;
static int mesh_layer = -1;
static esp_netif_t *netif_sta = NULL;

static EventGroupHandle_t s_wifi_event_group;

bool is_wifi_connected();

void _wifi_csi_cb(void *ctx, wifi_csi_info_t *data);

void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

void setup_station(void);

void setup_softap(void);

void mesh_scan_done_handler(int num);

void mesh_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

void ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

void ezmesh_init(void);

void setup_csi(void);
