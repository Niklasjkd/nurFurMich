#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_task.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "local_wifi.h"
#include "mqtt_client.h"
#include "esp_log.h"


#define CONFIG_BUSINESS "company1"
#define CONFIG_LOCATION "garage1"
#define CONFIG_ASSET "lot1"
#define CONFIG_MQTT_SELF "/" CONFIG_BUSINESS "/" CONFIG_LOCATION "/" CONFIG_ASSET
#define CONFIG_MQTT_GROUP "/" CONFIG_BUSINESS "/" CONFIG_LOCATION "/shared"
#define CONFIG_MQTT_PUBLISH CONFIG_MQTT_SELF "/publish"
#define CONFIG_WIFI_SSID "Airtel"
#define CONFIG_WIFI_PASSWORD "12345670"
#define CONFIG_WIFI_MAXIMUM_RETRY 8
#define WIFI_SUCCESS_BIT BIT0
#define WIFI_FAILURE_BIT BIT1
#define WIFI_NETWORK_BIT BIT2
#define MQTT_SUCCESS_BIT BIT3
#define MQTT_FAILURE_BIT BIT4
#define MQTT_CHANNEL_BIT BIT5
#define MQTT_UPGRADE_BIT BIT6
#define MQTT_CLEANUP_BIT BIT7

static void local_mqtt_init(void);
static void local_wifi_init(void);

static const char *MQTT_REMOTE = "wss://localhost:443";
//static const char *MQTT_LOCAL = "ws://failsafe.local:1883";

static const char *TAG = "local";
static EventGroupHandle_t local_events;
esp_mqtt_client_handle_t mqtt_client;
uint8_t mac_address[6] = {0xF1,0xF2,0xF3,0xF4,0xF5,0xF6};
static uint attempt = 0;
static void sta_handler(void* arg, esp_event_base_t base, int32_t id, void* data){
    if (id == WIFI_EVENT_STA_START){
        esp_wifi_connect();
    }else if (id == WIFI_EVENT_STA_DISCONNECTED){
        if (attempt < CONFIG_WIFI_MAXIMUM_RETRY){
            esp_wifi_connect();
            attempt++;
        }else{
            xEventGroupSetBits(local_events, WIFI_FAILURE_BIT);
        }
    }
}
static void ip_handler(void* arg, esp_event_base_t base, int32_t id, void* data){
    ip_event_got_ip_t* ip_event = (ip_event_got_ip_t*) data;
    attempt = 0;
    if (id == IP_EVENT_STA_GOT_IP || id == IP_EVENT_GOT_IP6){
        xEventGroupSetBits(local_events, WIFI_SUCCESS_BIT);
        local_mqtt_init();
    }else{
        ESP_LOGI(TAG,"Unknown IP Event : %d", (int)id);
    }
    ESP_LOGI(TAG, "Station IP = " IPSTR, IP2STR(&ip_event->ip_info.ip));
}
static void mqtt_handler(void* arg, esp_event_base_t base, int32_t id, void* data){
    esp_mqtt_event_handle_t handle = data;
    //esp_mqtt_client_handle_t client = handle->client;
    //int message_id;
    switch ((esp_mqtt_event_id_t)id){
    case MQTT_EVENT_CONNECTED:
        xEventGroupSetBits(local_events, MQTT_SUCCESS_BIT);
        xEventGroupClearBits(local_events, MQTT_FAILURE_BIT);
        break;
    case MQTT_EVENT_SUBSCRIBED:
        xEventGroupSetBits(local_events, MQTT_CHANNEL_BIT);
        break;
    case MQTT_EVENT_DATA:
        xEventGroupClearBits(local_events, MQTT_FAILURE_BIT);
        break;
    case MQTT_EVENT_PUBLISHED:
        xEventGroupClearBits(local_events, MQTT_FAILURE_BIT);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        xEventGroupClearBits(local_events, MQTT_CHANNEL_BIT);
        break;
    case MQTT_EVENT_DISCONNECTED:
        xEventGroupSetBits(local_events, MQTT_FAILURE_BIT);
        xEventGroupClearBits(local_events, MQTT_SUCCESS_BIT);
        break;
    default:
        ESP_LOGI(TAG,"Unknown MQTT Event : %d", handle->event_id);
        break;
    }
}
static void local_mqtt_init(void){
    esp_mqtt_client_config_t default_config = {
        .broker.address.uri = MQTT_REMOTE,
    };
    mqtt_client = esp_mqtt_client_init(&default_config);
    esp_mqtt_client_register_event(mqtt_client, MQTT_EVENT_ANY, mqtt_handler, mqtt_client);
    esp_mqtt_client_start(mqtt_client);
}
static void local_wifi_init(void){
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t default_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&default_config));
    esp_event_handler_instance_t sta_instance;
    esp_event_handler_instance_t ip_instance;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &sta_handler, NULL, &sta_instance));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, &ip_handler, NULL, &ip_instance));
    wifi_config_t sta_config = {
        .sta.ssid = CONFIG_WIFI_SSID,
        .sta.password = CONFIG_WIFI_PASSWORD,
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
    ESP_ERROR_CHECK(esp_wifi_get_mac(WIFI_IF_STA, mac_address));
    ESP_ERROR_CHECK(esp_wifi_start());
}
void local_wifi(void)
{
    printf("component_func");
    local_wifi_init();
}
