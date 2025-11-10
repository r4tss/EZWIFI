#include "EZWIFI.h"

/* void _wifi_csi_cb(void *ctx, wifi_csi_info_t *data) { */
/*     xSemaphoreTake(mutex, portMAX_DELAY); */
/*     std::stringstream ss; */

/*     wifi_csi_info_t d = data[0]; */
/*     char mac[20] = {0}; */
/*     sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", d.mac[0], d.mac[1], d.mac[2], d.mac[3], d.mac[4], d.mac[5]); */

/*     int data_len = data->len; */

/*     int8_t *my_ptr; */

/*     my_ptr = data->buf; */
/*     for (int i = 0; i < data_len; i++) { */
/*         //ss << (int) my_ptr[i] << " "; */
/*     } */

/*     my_ptr = data->buf; */
/*     for (int i = 0; i < data_len / 2; i++) { */
/*         //ss << (int) sqrt(pow(my_ptr[i * 2], 2) + pow(my_ptr[(i * 2) + 1], 2)) << " "; */
/*     } */
    
/*     my_ptr = data->buf; */
/*     for (int i = 0; i < data_len / 2; i++) { */
/*         //ss << (int) atan2(my_ptr[i*2], my_ptr[(i*2)+1]) << " "; */
/*     } */
/*     ss << "]\n"; */

/*     printf(ss.str().c_str()); */
/*     fflush(stdout); */
/*     vTaskDelay(0); */
/*     xSemaphoreGive(mutex); */
/* } */

void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if(event_id == WIFI_EVENT_AP_STACONNECTED) {
	wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
	ESP_LOGI(TAG, "station " MACSTR " join, AID=%d", MAC2STR(event->mac), event->aid);
    }
    else if(event_id == WIFI_EVENT_AP_STADISCONNECTED) {
	wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
	ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d", MAC2STR(event->mac), event->aid);
    }
    else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
	esp_wifi_connect();
    }
    else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
	ESP_LOGI(TAG, "Disconnected, retrying connection...");
	esp_wifi_connect();
	xEventGroupClearBits(s_wifi_event_group, BIT0);
    }
    else if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
	ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
	ESP_LOGI(TAG, "Got ip:" IPSTR, IP2STR(&event->ip_info.ip));
	xEventGroupSetBits(s_wifi_event_group, BIT0);
    }
}

void setup_station(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
							ESP_EVENT_ANY_ID,
							&wifi_event_handler,
							NULL,
							NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
							IP_EVENT_STA_GOT_IP,
							&wifi_event_handler,
							NULL,
							NULL));

    wifi_sta_config_t wifi_sta_config = {};
    wifi_sta_config.channel = WIFI_CHANNEL;
    wifi_config_t wifi_config = {
	.sta = wifi_sta_config,
    };

    strlcpy((char *) wifi_config.sta.ssid, SSID, sizeof(SSID));
    strlcpy((char *) wifi_config.sta.password, PASS, sizeof(PASS));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    esp_wifi_set_ps(WIFI_PS_NONE);

    ESP_LOGI(TAG, "Connecting to AP SSID:%s password:%s", SSID, PASS);
}

void setup_softap(void)
{
    s_wifi_event_group = xEventGroupCreate();
    
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
							ESP_EVENT_ANY_ID,
							&wifi_event_handler,
							NULL,
							NULL));

    wifi_ap_config_t wifi_ap_config = {};
    wifi_ap_config.channel = WIFI_CHANNEL;
    wifi_ap_config.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    wifi_ap_config.max_connection = MAX_STA_CONN;

    wifi_config_t wifi_config = {
	.ap = wifi_ap_config,
    };

    strlcpy((char *) wifi_config.ap.ssid, SSID, sizeof(SSID));
    strlcpy((char *) wifi_config.ap.password, PASS, sizeof(PASS));

    if(strlen(PASS) == 0) {
	wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "softap setup finished. SSID:%s password:%s", SSID, PASS);
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

void setup_csi(void) {
    ESP_ERROR_CHECK(esp_wifi_set_csi(1));

    wifi_csi_config_t configuration_csi;
    configuration_csi.lltf_en = 1;
    configuration_csi.htltf_en = 1;
    configuration_csi.stbc_htltf2_en = 1;
    configuration_csi.ltf_merge_en = 1;
    configuration_csi.channel_filter_en = 0;
    configuration_csi.manu_scale = 0;

    ESP_ERROR_CHECK(esp_wifi_set_csi_config(&configuration_csi));
    //ESP_ERROR_CHECK(esp_wifi_set_csi_rx_cb(&_wifi_csi_cb, NULL));

    //_print_csi_csv_header();

    ESP_LOGI(TAG, "setup csi finished.");
}
