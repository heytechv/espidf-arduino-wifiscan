/*
 * wifimode.cpp
 *
 * Created on Sun Oct 23 2022
 *
 * Copyright (c) 2022 Your Company
 */

#include <string>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/event_groups.h"

#include <esp_log.h>
#include <esp_err.h>

#include <nvs_flash.h>
#include <esp_wifi.h>
#include <esp_http_server.h>

#include "WifiProvisioning.h"

#define MODE_CONNECT_MAXIMUM_RETRY 10

static const char TAG[] = "WifiManager";


/**
 * @brief FreeRTOS event group to signal when we are connected
 * */
static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    /* Access point */
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "(wifi_event_handler) station "MACSTR" join, AID=%d", MAC2STR(event->mac), event->aid);

    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "(wifi_event_handler) station "MACSTR" leave, AID=%d", MAC2STR(event->mac), event->aid);
    }

    /* Connect (Wifi) */
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < MODE_CONNECT_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "(wifi_event_handler) retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_STA_FAIL_FLAG);
        }
        ESP_LOGI(TAG,"(wifi_event_handler) connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "(wifi_event_handler) got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_STA_CONNECTED_FLAG);
    }
}

/************************************************
 * Mode: AP
*************************************************/
void WifiProvisioning::initAP() {

    /* Init the tcp stack */
    ESP_ERROR_CHECK(esp_netif_init());

    /* Event loop for the wifi driver */
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_ap();

    /* Default wifi config */
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&config));
    // ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    /* Register handlers for WiFi/IP change */
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    // ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT  , ESP_EVENT_ANY_ID, &ipEventHandler, NULL, NULL));

    /* AP config */
    wifi_config_t ap_config = {
        .ap = {
            "ESP32-PixelClock",     // ssid
            "password",             // password
            {},                     // ssid_len
            0,                      // channel
            WIFI_AUTH_WPA_WPA2_PSK, // authmode
            0,                      // ssid_hidden
            2,                      // max_connection
            {},                     // beacon_interval
            {},                     // pairwise_cipher
            {}                      // ftm_responder
        }
    };

    /* Set mode, config and start */
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    ESP_LOGI(TAG, "(initAP) Access point started");
    wifi_event_callback(WIFI_AP_CREATED_FLAG);
}

/************************************************
 * Mode: Connect (connect to wifi saved in nvs)
*************************************************/
void WifiProvisioning::initSTA(std::string ssid, std::string pass) {
    s_wifi_event_group = xEventGroupCreate();

    /* Init the tcp stack */
    ESP_ERROR_CHECK(esp_netif_init());

    /* Event loop for the wifi driver */
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    /* Default wifi config */
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&config));

    /* Register handlers for WiFi/IP change */
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip));

    /* STA config */
    wifi_config_t wifi_config = {};

    char* myssid = new char[ssid.length()+1];
    strcpy(myssid, ssid.c_str());
    
    char *mypw = new char[pass.length()+1];
    strcpy(mypw, pass.c_str());

    strcpy((char *)wifi_config.sta.ssid, myssid);
    strcpy((char *)wifi_config.sta.password, mypw);

    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    /* Set mode, config and start */
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "(initConnect) Connect wifi init finished");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(
        s_wifi_event_group,
        WIFI_STA_CONNECTED_FLAG | WIFI_STA_FAIL_FLAG,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY
    );

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_STA_CONNECTED_FLAG) {
        ESP_LOGI(TAG, "(initConnect) connected to ap SSID:%s password:%s", ssid.c_str(), pass.c_str());
        wifi_event_callback(WIFI_STA_CONNECTED_FLAG);
    } else if (bits & WIFI_STA_FAIL_FLAG) {
        ESP_LOGI(TAG, "(initConnect) Failed to connect to SSID:%s, password:%s", ssid.c_str(), pass.c_str());
        wifi_event_callback(WIFI_STA_FAIL_FLAG);
    } else {
        ESP_LOGE(TAG, "(initConnect) UNEXPECTED EVENT");
        wifi_event_callback(WIFI_STA_FAIL_FLAG);
    }

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
}

