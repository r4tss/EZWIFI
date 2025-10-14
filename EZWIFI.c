#include "EZWIFI.h"

void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
  
}

void setup_station(void)
{
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  cfg.csi_enable = 1;
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

  esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
  assert(sta_netif);

  wifi_config_t wifi_config = {
    .sta = {
      .ssid = SSID,
      .password = PASS,
      .scan_method = WIFI_ALL_CHANNEL_SCAN,
      .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
      .threshold.rssi = -127,
      .threshold.authmode = WIFI_AUTH_WPA2_PSK,
      .threshold.rssi_5g_adjustment = 0,
    },
  };

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
}

void setup_ap(void)
{
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
}

void mesh_scan_done_handler(int num)
{
}

void mesh_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
}

void ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
  ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
  ESP_LOGI(TAG, "<IP_EVENT_STA_GOT_IP>IP:" IPSTR, IP2STR(&event->ip_info.ip));
}

void ezmesh_init(void)
{
  // Init networking, storage and event loop
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  ESP_ERROR_CHECK(esp_netif_create_default_wifi_mesh_netifs(&netif_sta, NULL));
  // Init WiFi
  wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();

  // TODO: Enable CSI HERE!
  
  ESP_ERROR_CHECK(esp_wifi_init(&config));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, NULL));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
  ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

  // Start WiFi
  ESP_ERROR_CHECK(esp_wifi_start());

  // Init mesh
  ESP_ERROR_CHECK(esp_mesh_init());
  ESP_ERROR_CHECK(esp_event_handler_register(MESH_EVENT, ESP_EVENT_ANY_ID, &mesh_event_handler, NULL));

  // Mesh config
  mesh_cfg_t cfg = MESH_INIT_CONFIG_DEFAULT();

  // Mesh ID
  memcpy((uint8_t *) &cfg.mesh_id, MESH_ID, 6);

  // Router
  cfg.channel = MESH_CHANNEL;
  cfg.router.ssid_len = strlen(MESH_ROUTER_SSID);
  memcpy((uint8_t *) &cfg.router.ssid, MESH_ROUTER_SSID, cfg.router.ssid_len);
  memcpy((uint8_t *) &cfg.router.password, MESH_ROUTER_PASS, strlen(MESH_ROUTER_PASS));

  // Mesh softAP
  ESP_ERROR_CHECK(esp_mesh_set_ap_authmode(MESH_AP_AUTHMODE));
  cfg.mesh_ap.max_connection = MESH_AP_CONNECTIONS;
  cfg.mesh_ap.nonmesh_max_connection = MESH_NON_MESH_AP_CONNECTIONS;
  memcpy((uint8_t *) &cfg.mesh_ap.password, MESH_AP_PASS, strlen(MESH_AP_PASS));
  ESP_ERROR_CHECK(esp_mesh_set_config(&cfg));

  ESP_LOGI(TAG, "<Config>disable IE crypto");

  ESP_ERROR_CHECK(esp_mesh_start());
  ESP_LOGI(TAG, "Mesh starts successfully, heap:%" PRId32, esp_get_free_heap_size());
}
