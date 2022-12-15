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


static const char TAG[] = "WifiProvisioning.AP";

extern const char webprovisioning_html_start[] asm("_binary_webprovisioning_html_start");
extern const char webprovisioning_html_end[] asm("_binary_webprovisioning_html_end");


static esp_err_t uri_get_root_handler(httpd_req_t *req) {
    size_t html_len = webprovisioning_html_end - webprovisioning_html_start;

    /* Send html */
    esp_err_t res = httpd_resp_send(req, webprovisioning_html_start, html_len);

    /* Status */
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Error httpd_resp_send: %d", res);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Success httpd_resp_send");
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
    const char resp[] = "<h1 style='position: absolute;top: 50%;left: 50%;transform: translateX(-50%) translateY(-50%);'>OK. Restarting...</h1>";
    httpd_resp_send(req, resp, strlen(resp));

    /* Restart */
    vTaskDelay(100 / portTICK_PERIOD_MS);

    setModeSTA();
    esp_restart();

    return ESP_OK;
}

static esp_err_t uri_post_time_handler(httpd_req_t *req) {

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
    ESP_LOGI(TAG, "Received: %s", content);

    /* Find time */
    char key_timeInSeconds[] = "timeInSeconds=";

    std::string timeInSeconds = WifiUtils::parseUri(content, key_timeInSeconds);
    ESP_LOGI(TAG, "timeInSeconds: %s", timeInSeconds.c_str());

    long seconds = stol(timeInSeconds);
    ESP_LOGI(TAG, "timeInSeconds (long): %ld", seconds);

    struct timeval tv = { .tv_sec = seconds, .tv_usec = 0, };
    settimeofday(&tv, NULL);

    /* Send a response */
    const char resp[] = "<h1 style='position: absolute;top: 50%;left: 50%;transform: translateX(-50%) translateY(-50%);'>OK. Restarting...</h1>";
    httpd_resp_send(req, resp, strlen(resp));

    /* Restart */
    vTaskDelay(100 / portTICK_PERIOD_MS);

    setModeSTA();
    esp_restart();

    return ESP_OK;
}

httpd_handle_t WifiProvisioning::startAPWebServer() {
    wifi_event_callback(WIFI_AP_WEBSERVER_STARTING_FLAG);

    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the http server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    esp_err_t res = httpd_start(&server, &config);

    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Error starting server %d", res);
        wifi_event_callback(WIFI_AP_FAIL_FLAG);
        return NULL;
    }

    /* Register URI handlers */
    httpd_uri_t uri_get_root = {
        .uri        = "/",
        .method     = HTTP_GET,
        .handler    = uri_get_root_handler
    };
    res = httpd_register_uri_handler(server, &uri_get_root);

    httpd_uri_t uri_post_root = {
        .uri        = "/",
        .method     = HTTP_POST,
        .handler    = uri_post_root_handler
    };
    res = httpd_register_uri_handler(server, &uri_post_root);

    httpd_uri_t uri_post_time = {
        .uri        = "/api/time",
        .method     = HTTP_POST,
        .handler    = uri_post_time_handler
    };
    res = httpd_register_uri_handler(server, &uri_post_time);

    ESP_LOGI(TAG, "Started.");
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

