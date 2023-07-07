#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "mqtt_client.h"
#include "local_wifi.h"
#define CONTENT 11
static const char *defaults[CONTENT][2] = {{"business","parking1"},{"location","garage1"},{"asset","lot1"},{"online","wss://io.abacus-ewall.de:443"},{"offline","ws://io.local:1883"},{"update","https://static.abacus-ewall.de:443/update"},{"ssid", "Airtel"},{"password","12345670"},{"retry","5"},{"crt","certificate"},{"key","private_key"}}; 
static const char *TAG = "local";
static void nvs_defaults(char local[]){
    nvs_handle_t handle;
    if (nvs_flash_init()==ESP_OK){
        if (nvs_open(TAG,NVS_READWRITE, &handle) == ESP_OK){
            for (size_t i = 0; i < CONTENT; i++){
                char *pair[] = local[i];
                nvs_set_str(handle, pair[0], pair[1]);
            }
            nvs_commit(handle);
        }
        nvs_close(handle);
    }
    
}
void app_main(void)
{
    esp_err_t nvs_error = nvs_flash_init();
    if (nvs_error == ESP_ERR_NVS_NO_FREE_PAGES || nvs_error == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        nvs_flash_erase();
        nvs_defaults(defaults);
    }
    ESP_ERROR_CHECK(nvs_error);
    nvs_handle_t handle;
    if (nvs_open("local",NVS_READONLY, &handle) == ESP_OK){
        /* code */
    }
    
    local_wifi();
}
