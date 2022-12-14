#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_log.h>
#include <esp_err.h>

#include <sys/param.h>

#include <nvs_flash.h>
#include <esp_wifi.h>
#include <esp_http_server.h>
#include "esp_sntp.h"

#include <driver/gpio.h>

#include <string>
#include <string.h>

#include "WifiProvisioning.h"
#include "nvs/nvsutils.h"
#include "wifiutils/wifiutils.h"

#include "esp_http_client.h"
#include "esp_tls.h"


#include "cJSON.h"

static const char TAG[] = "wifiprovisioning";


RTC_NOINIT_ATTR int MODE = 1;  // 0 = AP, 1 = STA

void WifiProvisioning::saveWifiInfoToNVS(WifiInfo_t *wifiInfo) {
    NVSUtils nvswifi(NVS_WIFI_NAMESPACE);

    // Init
    esp_err_t res = nvswifi.init();
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "(saveWifiInfoToNVS) Error init");
        return;
    }

    // Set data
    nvswifi.set(NVS_WIFI_KEY, (uint8_t*) wifiInfo, sizeof(WifiInfo_t));

    // Save all changes
    res = nvswifi.commit();
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "(saveWifiInfoToNVS) Error commit");
        return;
    }

    nvswifi.close();
}

int WifiProvisioning::readWifiInfoFromNVS(WifiInfo_t *wifiInfo) {
    NVSUtils nvswifi(NVS_WIFI_NAMESPACE);

    // Init
    esp_err_t res = nvswifi.init();
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "(readWifiInfoFromNVS) Error init");
        return res;
    }

    // Get Data
    size_t size = sizeof(WifiInfo_t);
    res = nvswifi.get(NVS_WIFI_KEY, (uint8_t*) wifiInfo, size);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "(readWifiInfoFromNVS) Error get");
        return res;
    }

    // Check data
    if (strlen(wifiInfo->ssid) == 0) {
        ESP_LOGI(TAG, "(readWifiInfoFromNVS) NULL ssid");
        return -1;
    }

    // Print wifi info
    ESP_LOGI(TAG, "(readWifiInfoFromNVS) WifiInfo.ssid = %s", wifiInfo->ssid);
    ESP_LOGI(TAG, "(readWifiInfoFromNVS) WifiInfo.pass = %s", wifiInfo->pass);

    nvswifi.close();

    return 0;
}

void WifiProvisioning::begin(void (*callback)(uint8_t flag)) {
    ESP_LOGI(TAG, "(run) >> wifiprovisioning");
	ESP_LOGI(TAG, "(run)  +------------------+");
	ESP_LOGI(TAG, "(run)  | WifiProvisioning |");
	ESP_LOGI(TAG, "(run)  +------------------+");

    wifi_event_callback = callback;

    // gpio init
    // gpio_pad_select_gpio(RESET_GPIO);
    // gpio_set_direction(RESET_GPIO, GPIO_MODE_INPUT);
    // gpio_set_pull_mode(RESET_GPIO, GPIO_PULLDOWN_ONLY);

    // to \/
    // gpio_pad_select_gpio(RESET_GPIO);
    // gpio_set_direction  ((gpio_num_t) RESET_GPIO, GPIO_MODE_INPUT);
    // gpio_pulldown_en    ((gpio_num_t) RESET_GPIO);
    // gpio_pullup_dis     ((gpio_num_t) RESET_GPIO);

    // Wifi info
    WifiInfo_t wifiInfo;
    int wifi_read_res = readWifiInfoFromNVS(&wifiInfo);    // read info

    // Check whether reset gpio pressed or WifiInfo not saved in NVS storage
    // if (gpio_get_level(RESET_GPIO) == 1 || wifi_read_res != ESP_OK) {
    if (MODE == 0) {

        // start access point and server, then save input data as new WifiInfo
        ESP_LOGI(TAG, "(begin) Mode = AP");

        // strcpy(wifiInfo.ssid, "XDD");
        // strcpy(wifiInfo.pass, "XDD");
        // saveWifiInfo(&wifiInfo);
        modeAP();
                
    } else {
        // connect to wifi using WifiInfo credentials
        ESP_LOGI(TAG, "(begin) Mode = connect");
        modeSTA(wifiInfo);
    }
}




void setModeAP() {
    MODE = 0;
}

void setModeSTA() {
    MODE = 1;
}

