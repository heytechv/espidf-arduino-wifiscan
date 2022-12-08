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


static const char TAG[] = "WifiProvisioning.STA";


static esp_err_t uri_get_root_handler(httpd_req_t *req) {
    // Send response (html)
    esp_err_t res = httpd_resp_send(req, (char*)modeAP_root_html, modeAP_root_html_len);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "(uri_get_root_handler) Error httpd_resp_send: %d", res);
        return -1;
    }

    ESP_LOGI(TAG, "(uri_get_root_handler) Success httpd_resp_send");
    return ESP_OK;
}

static esp_err_t uri_post_root_handler(httpd_req_t *req) {
    /* Documentation for HTTP POST:
     * https://docs.espressif.com/projects/esp-idf/en/v4.2.2/esp32/api-reference/protocols/esp_http_server.html */

    /* Destination buffer for content of HTTP POST request */
    char content[255];

    /* Truncate if content length larger than the buffer */
    size_t recv_size = MIN(req->content_len, sizeof(content));

    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {  /* 0 return value indicates connection closed */
        /* Check if timeout occurred */
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            /* In case of timeout we can retry httpd_req_recv(),
             * for now sent Timeout error*/
            httpd_resp_send_408(req);
        }
        /* In case of error, return ESP_FAIL to ensure
         * that connetion gets closed */
        return ESP_FAIL;
    }

    /* Print received content */
    ESP_LOGI(TAG, "(uri_post_root_handler) Received: %s", content);

    /* Find ssid and password */
    char key_ssid[] = "ssid=";
    char key_pass[] = "pass=";

    std::string ssid = WifiUtils::parseUri(content, key_ssid);
    std::string pass = WifiUtils::parseUri(content, key_pass);

    ESP_LOGI(TAG, "(uri_post_root_handler) ssid: %s, pass: %s", ssid.c_str(), pass.c_str());

    /* Save ssid and password to NVS */
    ESP_LOGI(TAG, "(uri_post_root_handler) Savind to NVS");
    WifiInfo_t wifiInfo;
    strcpy(wifiInfo.ssid, ssid.c_str());
    strcpy(wifiInfo.pass, pass.c_str());   
    WifiProvisioning::saveWifiInfoToNVS(&wifiInfo);

    /* Send a response */
    const char resp[] = "OK";
    httpd_resp_send(req, resp, strlen(resp));

    return ESP_OK;
}

static const httpd_uri_t uri_get_root = {
    .uri        = "/",
    .method     = HTTP_GET,
    .handler    = uri_get_root_handler
};
static const httpd_uri_t uri_post_root = {
    .uri        = "/",
    .method     = HTTP_POST,
    .handler    = uri_post_root_handler
};

httpd_handle_t WifiProvisioning::startAPWebServer() {
    wifi_event_callback(WIFI_AP_WEBSERVER_STARTING_FLAG);

    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the http server
    ESP_LOGI(TAG, "(startAPWebServer) Starting server on port: '%d'", config.server_port);
    esp_err_t res = httpd_start(&server, &config);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "(startAPWebServer) Error starting server %d", res);
        wifi_event_callback(WIFI_AP_FAIL_FLAG);
        return NULL;
    }

    // If ok register URI handlers
    res = httpd_register_uri_handler(server, &uri_get_root);
    res = httpd_register_uri_handler(server, &uri_post_root);

    ESP_LOGI(TAG, "(startAPWebServer) Started.");
    wifi_event_callback(WIFI_AP_WEBSERVER_STARTED_FLAG);
    return server;
}

void WifiProvisioning::stopAPWebServer(httpd_handle_t server) {
    httpd_stop(server);
}

void WifiProvisioning::modeAP() {
    wifi_event_callback(WIFI_AP_CREATING_FLAG);

    static httpd_handle_t server = NULL;
    initAP();

    server = startAPWebServer();
}

