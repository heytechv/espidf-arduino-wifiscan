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

// static const httpd_uri_t uri_sta_get_root = {
//     .uri        = "/",
//     .method     = HTTP_GET,
//     .handler    = uri_sta_get_root_handler
// };


void WifiProvisioning::modeSTA(WifiInfo_t wifiInfo) {
    wifi_event_callback(WIFI_STA_CONNECTING_FLAG);
    initSTA(wifiInfo.ssid, wifiInfo.pass);



    // static httpd_handle_t server = NULL;
    // server = startSTAWebServer();


    // Init sntp
    // sntp_setoperatingmode(SNTP_OPMODE_POLL);
    // sntp_setservername(0, "pool.ntp.org");
    // sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    // sntp_init();

    // xTaskCreate(&http_test_task, "http_test_task", 8192, NULL, 5, NULL);
}

// httpd_handle_t WifiProvisioning::startSTAWebServer() {
//     httpd_handle_t server = NULL;
//     httpd_config_t config = HTTPD_DEFAULT_CONFIG();

//     // Start the http server
//     ESP_LOGI(TAG, "(startSTAWebServer) Starting server on port: '%d'", config.server_port);
//     esp_err_t res = httpd_start(&server, &config);
//     if (res != ESP_OK) {
//         ESP_LOGE(TAG, "(startSTAWebServer) Error starting server %d", res);
//         return NULL;
//     }

//     // If ok register URI handlers
//     res = httpd_register_uri_handler(server, &uri_sta_get_root);

//     ESP_LOGI(TAG, "(startSTAWebServer) Done.");
//     return server;
// }

static esp_err_t uri_sta_get_root_handler(httpd_req_t *req) {

    // time_t now;
    // char strftime_buf[64];
    // struct tm timeinfo;

    // time(&now);

    // setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);  // Warsaw
    // tzset();

    // localtime_r(&now, &timeinfo);
    // strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);

    // Send response (html)
    // esp_err_t res = httpd_resp_send(req, strftime_buf, strlen(strftime_buf));

    char site[] = "xDD";

    // Send response (html)
    esp_err_t res = httpd_resp_send(req, site, strlen(site));
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "(uri_get_root_handler) Error httpd_resp_send: %d", res);
        return -1;
    }

    ESP_LOGI(TAG, "(uri_get_root_handler) Success httpd_resp_send");
    return ESP_OK;
}









void time_sync_notification_cb(struct timeval *tv) {

    char strftime_buf[64];
    struct tm timeinfo;

    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);  // Warsaw
    tzset();

    localtime_r(&tv->tv_sec, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "Notification of a time synchronization event %lld\n", (long long) tv->tv_sec);
    ESP_LOGI(TAG, "The current date/time in Shanghai is: %s", strftime_buf);
}















// Open weather map START


std::string local_response_buffer = "";
double today_temperature = -100;


esp_err_t client_event_get_handler(esp_http_client_event_handle_t evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        local_response_buffer.append((char *)evt->data);
        // printf("HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);
        break;

    default:
        break;
    }
    return ESP_OK;
}



















void rest_get()
{

    // char local_response_buffer[6000] = {0};
    
    esp_http_client_config_t config_get = {
        .url = "http://api.open-meteo.com/v1/forecast?latitude=50.28&longitude=18.81&daily=weathercode,apparent_temperature_max&timezone=Europe%2FLondon",
        .cert_pem = NULL,
        .method = HTTP_METHOD_GET,
        .event_handler = client_event_get_handler,};
        
    esp_http_client_handle_t client = esp_http_client_init(&config_get);
    esp_http_client_perform(client);

    printf("HTTP_EVENT_ON_DATA2: %.*s\n", 6000, local_response_buffer.c_str());

    cJSON *root2 = cJSON_Parse(local_response_buffer.c_str());


    cJSON *daily = cJSON_GetObjectItem(root2, "daily");
    cJSON *apparent_temperature_max = cJSON_GetObjectItem(daily, "apparent_temperature_max");

    double today_temp = cJSON_GetArrayItem(apparent_temperature_max, 0)->valuedouble;

    today_temperature = today_temp;


    printf("XD_DANE %lf", today_temp);

    // char *daily = cJSON_GetObjectItem(apparent_temperature_max, "daily")->valuestring;





    esp_http_client_cleanup(client);
}


static void http_test_task(void *pvParameters) {
    // rest_get();
    vTaskDelete(NULL);
}


// Open weather map END










