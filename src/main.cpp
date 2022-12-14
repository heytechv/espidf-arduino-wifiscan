/* WiFi scan Example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/queue.h"

#include <esp_log.h>
#include <esp_err.h>

#include "sdkconfig.h"
#include <Arduino.h>
#include <WiFi.h>
#include "main.h"
#include "wifiprovisioning/WifiProvisioning.h"
#include "display/display.h"

#include "cJSON.h"
// #include "screens/screens.h"

// todo register plugin
// #include "screens/ScreenSetup.h"
// #include "screens/ScreensRob.h"

#include "esp_sntp.h"

#include <time.h>
#include <sys/time.h>

#include "esp_spiffs.h"

#include "screens/ScreensManager.h"
#include "EasyButton/EasyButton.h"
#include "esp_spiffs.h"

#include <sys/param.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#include "display/ws2812b/ws2812b.h"


#define INPUT_PIN 25

static const char TAG[] = "Main";



extern const char webmain_html_start[] asm("_binary_webmain_html_start");
extern const char webmain_html_end[] asm("_binary_webmain_html_end");


void wifiScan() {
    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n == 0) {
        Serial.println("no networks found");
    } else {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
            delay(10);
        }
    }
    Serial.println("");
}


void arduinoTask(void *pvParameter) {
    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    Serial.begin(115200);
    delay(100);

    while(1) {
        wifiScan();

        // Wait a bit before scanning again
        delay(5000);
    }
}

/**
 * Webserver
 */
static esp_err_t uri_get_root_handler(httpd_req_t *req) {
    size_t html_len = webmain_html_end - webmain_html_start;

    /* Send html */
    esp_err_t res = httpd_resp_send(req, webmain_html_start, html_len);

    /* Status */
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Error httpd_resp_send: %d", res);
        return ESP_FAIL;
    }

    return ESP_OK;

}

static esp_err_t uri_get_screens_handler(httpd_req_t *req) {
    /**
     * {
     *  "screen_name": [
     *      [
     *          1,
     *          "Main graphic",
     *          "Graphic displayed next to time",
     *          "skull0.bmp"
     *      ],
     *      [
     *          0,
     *          "Timezone",
     *          "Your city timezone",
     *          "Warsaw"
     *      ]
     *  ]
     * }
     */

    /* Get list of screens and their config */
    std::string screensJson = screensManager_get_config_json_str();

    /* Set header to json */
    httpd_resp_set_type(req, "application/json");

    /* Send list */
    esp_err_t res = httpd_resp_send(req, (char*)screensJson.c_str(), screensJson.length());

    /* Status */
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Error httpd_resp_send: %d", res);
        return ESP_FAIL;
    }

    return ESP_OK;
}

static esp_err_t uri_get_files_handler(httpd_req_t *req) {
    /**
     * filename1,filename2...
     */
    esp_err_t res;

    std::string sem = ",";

    DIR* dir = opendir("/spiffs/");
    if (dir == NULL) {
        return ESP_FAIL;
    }

    while (true) {
        struct dirent* de = readdir(dir);
        if (!de) {
            break;
        }

        res = httpd_resp_sendstr_chunk(req, de->d_name);
        res = httpd_resp_sendstr_chunk(req, sem.c_str());
        
        // printf("Found file: %s\n", de->d_name);
    }

    closedir(dir);

    /* Send empty chunk to signal HTTP response completion */
    httpd_resp_sendstr_chunk(req, NULL);

    return ESP_OK;
}

static esp_err_t uri_get_screens_visible_amount_handler(httpd_req_t *reg) {
    int amount = screensManager_get_screens_visible_amount();
    std::string strAmount = std::to_string(amount);
    httpd_resp_sendstr(reg, strAmount.c_str());
    return ESP_OK;
}

static esp_err_t uri_post_config_screen_handler(httpd_req_t *req) {
    /**
     *      [ "screen_name", "conf1_val" , "conf2_val" ...]
     * e.g. [ "Time"       , "clock1.bmp", "10:00"]
     */

    int total_len = req->content_len;

    int cur_len = 0;
    int received_len = 0;

    char buf[255];

    /* Read data */
    while (cur_len < total_len) {
        received_len = httpd_req_recv(req, buf + cur_len, total_len);

        if (received_len <= 0) {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
            return ESP_FAIL;
        }

        cur_len += received_len;
    }
    buf[total_len] = '\0';

    ESP_LOGI(TAG, "%s", buf);

    /* Parse JSON */
    cJSON *root = cJSON_Parse(buf);
    int root_size = cJSON_GetArraySize(root);

    std::string screen_name;
    std::vector<std::string> config;

    // if no screen_name
    if (root_size == 0) {
        httpd_resp_sendstr(req, "Json error (length=0)");
        return ESP_FAIL;
    }
    screen_name = cJSON_GetArrayItem(root, 0)->valuestring;

    // get config
    for (int i = 1; i<root_size; i++) {
        cJSON *el = cJSON_GetArrayItem(root, i);
        config.push_back(el->valuestring);
    }

    esp_err_t err = screensManager_set_screen(screen_name, config);

    if (err == ESP_OK) httpd_resp_sendstr(req, "OK");
    else httpd_resp_sendstr(req, "Failed");

    cJSON_Delete(root);

    return ESP_OK;
}

/**
 * @brief Control which screens and in what order will be displayed
 */
static esp_err_t uri_post_config_screens_handler(httpd_req_t *req) {
    /**
     * ["screen_name1", "screen_name2"...]
     */

    int total_len = req->content_len;

    int cur_len = 0;
    int received_len = 0;

    char buf[255];

    /* Read data */
    while (cur_len < total_len) {
        received_len = httpd_req_recv(req, buf + cur_len, total_len);

        if (received_len <= 0) {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post config value");
            return ESP_FAIL;
        }

        cur_len += received_len;
    }
    buf[total_len] = '\0';

    ESP_LOGI(TAG, "%s", buf);

    /* Parse JSON */
    cJSON *root = cJSON_Parse(buf);
    int root_size = cJSON_GetArraySize(root);

    std::string screen_name;
    std::vector<std::string> config;

    // if 0 screens
    if (root_size == 0) {
        httpd_resp_sendstr(req, "Json error (length=0)");
        return ESP_FAIL;
    }

    // get screens
    for (int i = 0; i<root_size; i++) {
        cJSON *el = cJSON_GetArrayItem(root, i);
        config.push_back(el->valuestring);
    }

    esp_err_t err = screensManager_set_screens(config);

    if (err == ESP_OK) httpd_resp_sendstr(req, "OK");
    else httpd_resp_sendstr(req, "Failed");

    cJSON_Delete(root);

    return ESP_OK;
}

static void start_web_server() {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the http server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    esp_err_t res = httpd_start(&server, &config);

    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Error starting server %d", res);
        return;
    }

    /* URI */
    httpd_uri_t uri_get_root = {
        .uri        = "/",
        .method     = HTTP_GET,
        .handler    = uri_get_root_handler
    };
    httpd_register_uri_handler(server, &uri_get_root);

    httpd_uri_t uri_get_files = {
        .uri        = "/api/files",
        .method     = HTTP_GET,
        .handler    = uri_get_files_handler
    };
    httpd_register_uri_handler(server, &uri_get_files);

    httpd_uri_t uri_get_screens = {
        .uri        = "/api/screens",
        .method     = HTTP_GET,
        .handler    = uri_get_screens_handler
    };
    httpd_register_uri_handler(server, &uri_get_screens);

    httpd_uri_t uri_get_screens_visible_amount = {
        .uri        = "/api/screens/visible-amount",
        .method     = HTTP_GET,
        .handler    = uri_get_screens_visible_amount_handler
    };
    httpd_register_uri_handler(server, &uri_get_screens_visible_amount);

    httpd_uri_t uri_post_config_screen = {
        .uri        = "/api/screen",
        .method     = HTTP_POST,
        .handler    = uri_post_config_screen_handler
    };
    httpd_register_uri_handler(server, &uri_post_config_screen);

    httpd_uri_t uri_post_config_screens = {
        .uri        = "/api/screens",
        .method     = HTTP_POST,
        .handler    = uri_post_config_screens_handler
    };
    httpd_register_uri_handler(server, &uri_post_config_screens);

}

/**
 * onWifiEvent
 */
static void onWifiEvent(uint8_t flag) {
    ESP_LOGI(TAG, "Wifi_Event: %d", flag);

    // Send data to screens
    xQueueSend(screens_manager_wifi_queue, &flag, (TickType_t)0);

    // Start webserver (on wifi connected event)
    if (flag == WIFI_STA_CONNECTED_FLAG) {
        start_web_server();
    }
}

/**
 * onButtonNextClick
 */
static void onButtonNextClick(ButtonState &buttonState) {
    ESP_LOGI(TAG, "Button_Event NEXT: %d", buttonState);
    xQueueSend(screens_manager_btn_queue, (uint8_t*) &buttonState, (TickType_t)0);
}

/**
 * onButtonModeClick
 */
static void onButtonModeClick(ButtonState &buttonState) {
    ESP_LOGI(TAG, "Button_Event MODE: %d", buttonState);
    
    if (buttonState == SINGLE_CLICK) {
        ESP_LOGI(TAG, "Button_Event MODE = STA");
        setModeSTA();
        esp_restart();

    } else if (buttonState == DOUBLE_CLICK) {
        ESP_LOGI(TAG, "Button_Event MODE = AP");
        setModeAP();
        esp_restart();
    }
    
}

/**
 * Main
 */
extern "C" void app_main() {
    /* Cpu freq = 240MHz */
    setCpuFrequencyMhz(240);
    ESP_LOGI("XF", "CpuFreqMHz: %u MHz\n", getCpuFrequencyMhz());


    // CRGB leds[256]; //24 led
	// initSPIws2812();
	// for(int i = 0 ; i < 12 ; i++)
	// {
	// 	CRGB c;// = {.r=0,.g=0xff,.b=0};
    //     c.r = 0;
    //     c.g = 0;
    //     c.b = 238;
	// 	leds[i] = c;
	// }

    // for(int i = 12 ; i < 256 ; i++)
	// {
	// 	CRGB c;// = {.r=0,.g=0xff,.b=0};
    //     c.r = 120;
    //     c.g = 0;
    //     c.b = 0;
	// 	leds[i] = c;
	// }
	
	// fillBuffer((uint32_t*)&leds,256);
	// led_strip_update();

    // Display *di = new Display();
    // di->init();
    // di->setCursor(0);
    // di->print("XDDDDD");
    // di->sendBuffer();

    // while (1) {
    //     vTaskDelay(20 / portTICK_PERIOD_MS);
    // }


    /* initialize arduino library before we start the tasks */
    // initArduino();

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    /* Init spiffs */
    esp_vfs_spiffs_conf_t config_spiffs = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true,
    };
    esp_vfs_spiffs_register(&config_spiffs);

    /* WS2812B */
    // gpio_pad_select_gpio(GPIO_NUM_27);  // configure pins as GPIO
    // gpio_set_direction(GPIO_NUM_27, GPIO_MODE_OUTPUT);

    /* Screens */
    screens_manager_wifi_queue = xQueueCreate(10, sizeof(uint8_t));
    screens_manager_btn_queue = xQueueCreate(10, sizeof(ButtonState));
    screensManager_start();

    /* Button next */
    EasyButton btnNext;
    btnNext.setOnClickListener(onButtonNextClick);
    btnNext.begin(25);

    /* Button mode */
    EasyButton btnMode;
    btnMode.setOnClickListener(onButtonModeClick);
    btnMode.begin(GPIO_NUM_32);

    /* Init wifi provisioning */
    WifiProvisioning WifiProvisioningImpl;
    WifiProvisioningImpl.begin(&onWifiEvent);

    /* Init sntp */
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    // sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_init();

    /* While true */
    while (1) {
        btnNext.loop();
        btnMode.loop();
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }

}

