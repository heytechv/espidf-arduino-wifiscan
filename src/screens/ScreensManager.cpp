#include "ScreensManager.h"

#include <stdio.h>
#include <stdlib.h>

#include "cJSON.h"

#include <esp_log.h>
#include <esp_err.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/queue.h"
#include "wifiprovisioning/WifiProvisioning.h"

#include "EasyButton/EasyButton.h"

#include "Screens.h"

#include "default/ScreenLoading.h"
#include "default/ScreenAPConfig.h"
#include "default/ScreenSTAConnecting.h"
#include "default/ScreenSTAFailed.h"
#include "default/ScreenSTAConnected.h"

#include "extra/ScreenTime.h"
#include "extra/ScreenText.h"


static const char TAG[] = "ScreensManager";

/* List of all screens */
typedef struct ScreenConfig_t {
    Screens *screen;
    std::vector<Screens::ConfigInput_t> conf;
} ScreenConfig_t;

std::vector<ScreenConfig_t> screenList;  // the list of visible screens
int screenList_visible_amount = 4;       // how many screens are visible           

/* Ticks */
uint16_t screens_manager_ticks = 0;

/* Display */
Display *screens_manager_display;

/* Queues */
QueueHandle_t screens_manager_wifi_queue;
QueueHandle_t screens_manager_btn_queue;


/**
 * Init
 */
void screensManager_init() {
    /* Screen: Time */
    ScreenTime *screenTime = new ScreenTime();
    screensManager_register_screen(screenTime);

    /* Screen text*/
    ScreenText *screenText = new ScreenText();
    screensManager_register_screen(screenText);
}


/**
 * Screen registration
 */
esp_err_t screensManager_register_screen(Screens *s) {
    if (screenList.size() >= 255) {
        ESP_LOGE(TAG, "Unable to register screen, limit reached");
        return ESP_FAIL;
    }

    ScreenConfig_t sc = { s, s->getDefaultConfig() };
    screenList.push_back(sc);

    return ESP_OK;
}

/**
 * Start task
 */
void screensManager_start() {
    /* Init display */
    screens_manager_display = new Display();
    screens_manager_display->init();
    screens_manager_display->setBrightness(5);

    /* Init screens */
    screensManager_init();

    /* Start task */
    xTaskCreatePinnedToCore(screensManager_task, "screensManager_task", 8192, NULL, 3, NULL, APP_CPU_NUM);
}

/**
 * The task
 */
void screensManager_task(void *argp) {

    Screens *s = new ScreenLoading();
    ScreenConfig_t sc = {};

    /* Default immutable screens */
    Screens *sLoading = new ScreenLoading();

    Screens *sAPConfig = new ScreenAPConfig();

    Screens *sSTAConnecting = new ScreenSTAConnecting();
    Screens *sSTAFailed = new ScreenSTAFailed();
    Screens *sSTAConnected = new ScreenSTAConnected();

    uint8_t wait_ticks = 0;

    /* Loop */
    bool is_init_done = false;
    uint8_t cur_screen_index = 0;

    int8_t wifi_flag = 0;
    ButtonState btn_next_flag = IDLE;

    while (true) {
        wifi_flag = -1;
        btn_next_flag = IDLE;

        /* Ticks counting */
        screens_manager_ticks ++;
        if (screens_manager_ticks > 65000) screens_manager_ticks = 0;  // tick overflow

        /* Receive queues */
        if (xQueueReceive(screens_manager_wifi_queue, &wifi_flag, (TickType_t)0)) { }
        if (xQueueReceive(screens_manager_btn_queue, &btn_next_flag, (TickType_t)0)) { }

        /**
         *  Pre-built screens (init sequence)
         * */
        if (is_init_done == false) {
            /* AP */
            if (wifi_flag == WIFI_AP_CREATING_FLAG || wifi_flag == WIFI_AP_CREATED_FLAG || wifi_flag == WIFI_AP_WEBSERVER_STARTING_FLAG) {
                s = sLoading;
            } else if (wifi_flag == WIFI_AP_WEBSERVER_STARTED_FLAG) {
                s = sAPConfig;
            }
            /* STA */
            else if (wifi_flag == WIFI_STA_CONNECTING_FLAG) {
                s = sSTAConnecting;
            } else if (wifi_flag == WIFI_STA_FAIL_FLAG) {
                // Show connection failed screen for 5 seconds and continue without wifi
                s = sSTAFailed;
                wait_ticks = 50;
                is_init_done = true;
            } else if (wifi_flag == WIFI_STA_CONNECTED_FLAG) {
                // Show connection success screen for 15 seconds and continue with wifi
                s = sSTAConnected;
                wait_ticks = 10;
                is_init_done = true;
            }
        }

        // wait ticks (cant change screen)
        if (wait_ticks > 0) {
            wait_ticks --;
        }

        /**
         * Screen selecting
         */
        if (is_init_done && wait_ticks == 0) {
            
            /* Button */
            if (btn_next_flag == SINGLE_CLICK) {
                ESP_LOGI(TAG, "next screen");
                cur_screen_index ++;
            } else if (btn_next_flag == DOUBLE_CLICK || btn_next_flag == MULTI_CLICK) {
                ESP_LOGI(TAG, "screen action");
            }

            /* Overflow */
            if (cur_screen_index >= screenList.size() || cur_screen_index >= screenList_visible_amount) {
                cur_screen_index = 0;
            }

            /* Tick selected screen */
            sc = screenList.at(cur_screen_index);
            s = sc.screen;
        }

        s->tick(screens_manager_display, screens_manager_ticks, sc.conf);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

}

/**
 * Get config from each screen and return as json string
 */
std::string screensManager_get_config_json_str() {
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

    cJSON *jsonRoot, *jsonScreenArray, *jsonConfArray, *element;
    jsonRoot = cJSON_CreateObject();

    int screens_size = screenList.size();
    int conf_size = 0;

    ScreenConfig_t *sc;

    std::string screen_name;
    std::string conf_name;
    std::string fieldName; int fieldType; std::string title; std::string description; std::string value;

    /* Iterate over screens */
    for (int i=0; i<screens_size; i++) {
        sc = &screenList.at(i);

        screen_name = sc->screen->getName();
        conf_size = sc->conf.size();

        jsonScreenArray = cJSON_AddArrayToObject(jsonRoot, screen_name.c_str());

        /* Iterate over conf list of screen */
        for (int n=0; n<conf_size; n++) {
            fieldName = sc->conf.at(n).fieldName;

            fieldType = sc->conf.at(n).fieldType;
            title = sc->conf.at(n).title;
            description = sc->conf.at(n).description;
            value = sc->conf.at(n).value;

            jsonConfArray = cJSON_AddArrayToObject(jsonScreenArray, fieldName.c_str());

            element = cJSON_CreateNumber(fieldType);
            cJSON_AddItemToArray(jsonConfArray, element);

            element = cJSON_CreateString(title.c_str());
            cJSON_AddItemToArray(jsonConfArray, element);
            
            element = cJSON_CreateString(description.c_str());
            cJSON_AddItemToArray(jsonConfArray, element);

            element = cJSON_CreateString(value.c_str());
            cJSON_AddItemToArray(jsonConfArray, element);
        }
    }

    /* Parse to string */
    char *res = cJSON_PrintUnformatted(jsonRoot);
    cJSON_Delete(jsonRoot);
    return std::string(res);
}


/**
 * Set screen config (find by name)
 */
// UWAGA! przepisac na queue albo semaphore
esp_err_t screensManager_set_screen(std::string screen_name, std::vector<std::string> conf_new) {
    ScreenConfig_t *sc;

    std::string name;

    size_t screenList_size = screenList.size();
    int old_conf_size = 0, new_conf_size = 0, min_conf_size = 0;

    /* Find screen by name */
    for (int i=0; i<screenList_size; i++) {
        sc = &screenList.at(i);

        name = sc->screen->getName();
        old_conf_size = sc->conf.size();

        /* Input conf size */
        new_conf_size = conf_new.size();
        
        /* Minimum size (if there are missing params so skip them) */
        min_conf_size = old_conf_size;
        if (new_conf_size < min_conf_size) { min_conf_size = new_conf_size;}

        /* Update old conf with new (up to min size) */
        if (name == screen_name) {
            /* Update config */
            for (int n=0; n<min_conf_size; n++) {
                sc->conf.at(n).value = conf_new.at(n);
            }
            return ESP_OK;
        }
    }

    ESP_LOGE(TAG, "Unable to set conf for screen %s", screen_name.c_str());
    return ESP_FAIL;
}

/**
 * Set which screens and in what order will be displayed
 */
// UWAGA! przepisac na queue albo semaphore
esp_err_t screensManager_set_screens(std::vector<std::string> screenNames) {
    size_t screenList_size = screenList.size();
    size_t screenNames_size = screenNames.size();

    /* Size of screens to set cannot be larger than total screen amount*/
    if (screenNames_size > screenList_size) {
        ESP_LOGE(TAG, "Unable to set screens order, screenNames_size cannot be larger than screenList_size");
        return ESP_FAIL;
    }

    /* Init */
    ScreenConfig_t sc;
    ScreenConfig_t scTemp;
    std::string name, screen_name;

    /* Find screen by name */
    for (int i=0; i<screenList_size; i++) {
        sc = screenList.at(i);
        name = sc.screen->getName();

        for (int i2=0; i2<screenNames_size; i2++) {
            screen_name = screenNames.at(i2);
            
            if (name == screen_name) {
                // if is already on place, skip
                if (i == i2) {
                    break;
                }

                // Swap place
                scTemp = screenList.at(i2);
                screenList.at(i2) = sc;
                screenList.at(i) = scTemp;
                break;
            }
        }

    }
    screenList_visible_amount = screenNames.size();

    ESP_LOGE(TAG, "Screens order set");
    return ESP_OK;
}

int screensManager_get_screens_visible_amount() {
    return screenList_visible_amount;
}

