/*
 * wifiprovisioning.h
 *
 * Created on 
 *
 * Copyright (c) 2022 Your Company
 */

#ifndef _WIFIPROVISIONING_H_
#define _WIFIPROVISIONING_H_

#include <esp_http_server.h>
#include "freertos/event_groups.h"
#include <functional>

#define RESET_GPIO GPIO_NUM_32                   // GPIO used as reset signal (even if WifiInfo saved start AP for data input)

#define SSID_LENGTH       (32)                  // Maximum SSID size
#define PASSWORD_LENGTH   (64)                  // Maximum password size

#define NVS_WIFI_NAMESPACE  "wifispace"         // Namespace in NVS for bootwifi
#define NVS_WIFI_KEY        "wifidata"          // Key used in NVS for connection info


/**
 * @brief info structure (ssid and password)
 */
typedef struct WifiInfo_t {
    char ssid[SSID_LENGTH];
    char pass[PASSWORD_LENGTH];
} WifiInfo_t;

/**
 * Mode AP Site
*/
const unsigned char modeAP_root_html[] = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Select Wifi</title><meta name='viewport' content='width=device-width, initial-scale=1' /><style>html {font-family: Arial;}</style></head><body><form method='post' action='/'><input type='text' name='ssid' placeholder='SSID'/><br/><input type='text' name='pass' placeholder='Password'/><br/><input type='hidden' name='rob' value='rob'/><br/><input type='submit' value='Save'/></form></body>";
const int32_t modeAP_root_html_len = 443;




extern void setModeAP();

extern void setModeSTA();


extern double today_temperature;

void rest_get();


static void http_test_task(void *pvParameters);


enum {
    WIFI_STA_CONNECTING_FLAG = 0,
    WIFI_STA_CONNECTED_FLAG,
    WIFI_STA_FAIL_FLAG,

    WIFI_AP_CREATING_FLAG,
    WIFI_AP_CREATED_FLAG,
    WIFI_AP_FAIL_FLAG,
    WIFI_AP_WEBSERVER_STARTING_FLAG,
    WIFI_AP_WEBSERVER_STARTED_FLAG,
    WIFI_SKIP
    // WIFI_AP_CONFIGURED_STA_FLAG  // todo
};


class WifiProvisioning {

private:

    void initAP();
    void initSTA(std::string ssid, std::string pass);


    /**
     * @brief Mode: AP - Init
     */
    void modeAP();

    /**
     * @brief Mode: AP - Start server with input for SSID and password
     */
    httpd_handle_t startAPWebServer();

    /**
     * @brief Mode: AP - Stop server
     */
    void stopAPWebServer(httpd_handle_t server);

    /**
     * @brief Mode: STA - Connect to network using credentials from NVS
     */
    void modeSTA(WifiInfo_t wifiInfo);

    //void (*event_handler_func_global)(uint8_t);

public:

    void (*wifi_event_callback)(uint8_t flag);

    /**
     * @brief Save wifi data, ssid and password to NVS storage
     * 
     * @param[out] wifiInfo The wifi info data to save
     */
    static void saveWifiInfoToNVS(WifiInfo_t *wifiInfo);

    /**
     * @brief Read saved wifi data (if exists), ssid and password from NVS storage
     * 
     * @param[out] wifiInfo The pointer to result
     */
    static int readWifiInfoFromNVS(WifiInfo_t *wifiInfo);

    /**
     * @brief Begin WifiProvisioning
    */
    void begin(void (*callback)(uint8_t flag));

};



#endif