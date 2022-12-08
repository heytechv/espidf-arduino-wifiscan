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
#include "default/ScreenTime.h"
#include "default/ScreenText.h"
#include "default/ScreenSTAFailed.h"




/* Basic (not changeable) screens */
#define SCREEN_LOADING 0
#define SCREEN_AP_CREATED 1
#define SCREEN_STA_FAILED 2


static const char TAG[] = "ScreensManager";



typedef struct ScreenConfig_t {
    Screens *screen;
    std::vector<Screens::ConfigInput_t> conf;
} ScreenConfig_t;

std::vector<ScreenConfig_t> screenList;


uint16_t screens_manager_ticks = 0;

Display *screens_manager_display;

/* Queues */
QueueHandle_t screens_manager_wifi_queue;
QueueHandle_t screens_manager_btn_queue;

/* Screens */
// Screens *screensTab[255];
// unsigned int screensTab_amount = 0;
// unsigned int screensTab_basic_max = 0;
// unsigned int screensTab_index = 0;


void screens_manager_init() {
    /* Init display */
    screens_manager_display = new Display();
    screens_manager_display->init();

    screens_manager_display->setBrightness(5);

    /**
     * Register basic screens
     * */
    /* Screen: Loading */
    // ScreenLoading *screenLoading = new ScreenLoading();
    // screens_manager_register_basic_screen(screenLoading, SCREEN_LOADING);
    // /* Screen: AP created */
    // ScreenAPConfig *screenAPConfig = new ScreenAPConfig();
    // screens_manager_register_basic_screen(screenAPConfig, SCREEN_AP_CREATED);
    // /* Screen: STA failed */
    // ScreenSTAFailed *screenSTAFailed = new ScreenSTAFailed();
    // screens_manager_register_basic_screen(screenSTAFailed, SCREEN_STA_FAILED);

    // screensTab_amount = 3;
    // screensTab_basic_max = SCREEN_STA_FAILED;

    // /* Screen: Time */
    // ScreenTime *screenTime = new ScreenTime();
    // screens_manager_register_screen(screenTime);
    // /* Screen: Text */
    // ScreenText *screenText = new ScreenText();
    // screens_manager_register_screen(screenText);

    /* Screen: Time */
    ScreenTime *screenTime = new ScreenTime();
    screens_manager_register_screen(screenTime);

}

void screens_manager_start() {
    xTaskCreatePinnedToCore(screens_manager_task, "screens_manager_task", 8192, NULL, 3, NULL, APP_CPU_NUM);
}

void screens_manager_register_basic_screen(Screens *screensRob, uint8_t pos) {
    // if (pos >= 255) {
    //     ESP_LOGE(TAG, "Unable to register screen, limit reached");
    //     return;
    // }

    // screensTab[pos] = screensRob;

}

void screens_manager_register_screen(Screens *s) {
    if (screenList.size() >= 255) {
        ESP_LOGE(TAG, "Unable to register screen, limit reached");
        return;
    }

    ScreenConfig_t sc = { s, s->getDefaultConfig() };
    screenList.push_back(sc);
}

bool is_init_done = false;

void screens_manager_task(void *argp) {

    uint8_t wifi_flag = 0;
    ButtonState btn_next_flag = IDLE;

    uint8_t cur_screen_index = 0;

    while (true) {
        btn_next_flag = IDLE;

        /* Ticks counting */
        screens_manager_ticks ++;
        if (screens_manager_ticks > 65000) screens_manager_ticks = 0;  // tick overflow

        /* Receive queues */
        if (xQueueReceive(screens_manager_wifi_queue, &wifi_flag, (TickType_t)0)) { }
        if (xQueueReceive(screens_manager_btn_queue, &btn_next_flag, (TickType_t)0)) { }

        /**
         *  Pre-built screens 
         * */
        // if (wifi_flag == WIFI_AP_CREATING_FLAG || wifi_flag == WIFI_AP_CREATED_FLAG || wifi_flag == WIFI_AP_WEBSERVER_STARTING_FLAG) {
        //     /* Screen loading */
        //     screensTab_index = SCREEN_LOADING;
        // } else if (wifi_flag == WIFI_AP_WEBSERVER_STARTED_FLAG) {
        //     /* Screen AP created (website for input data)*/
        //     screensTab_index = SCREEN_AP_CREATED;
        // } else if (wifi_flag == WIFI_STA_CONNECTING_FLAG) {
        //     /* Screen loading */
        //     screensTab_index = SCREEN_LOADING;
        // } else if (wifi_flag == WIFI_STA_FAIL_FLAG) {
        //     /* Screen STA connection failed */
        //     screensTab_index = SCREEN_STA_FAILED;
        // } else if (wifi_flag == WIFI_STA_CONNECTED_FLAG) {
        //     is_init_done = true;
        //     screensTab_index = screensTab_basic_max+1;
        // }


        /**
         * User screens
         * */
        // if (is_init_done) {

        //     if (btn_next_flag == SINGLE_CLICK) {
        //         ESP_LOGI(TAG, "next screen");
        //         screensTab_index ++;
        //     } else if (btn_next_flag == DOUBLE_CLICK || btn_next_flag == MULTI_CLICK) {
        //         ESP_LOGI(TAG, "screen action");
        //     }


        // }


        /* Overflow */
        if (cur_screen_index > screenList.size()) {
            cur_screen_index = 0;
        }

        /** 
         * Tick selected screen
         * */
        // if (screensTab_index >= screensTab_amount) {
        //     screensTab_index = screensTab_basic_max+1;
        // }
        ScreenConfig_t sc = screenList.at(cur_screen_index);
        Screens *s = sc.screen;
        s->tick(screens_manager_display, screens_manager_ticks, sc.conf);

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

}

std::string screensManager_get_config_json_str() {
    /**
     * {
     *  "screen_name": {
     *      "conf_name1": [
     *          1,
     *          "Main graphic",
     *          "Graphic displayed next to time",
     *          "skull0.bmp"
     *      ],
     *      "conf_name2": [
     *          0,
     *          "Timezone",
     *          "Your city timezone",
     *          "Warsaw"
     *      ]
     *  }
     * }
     */

    cJSON *jsonRoot, *jsonScreen, *jsonConfArray, *element;
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

        cJSON_AddItemToObject(jsonRoot, screen_name.c_str(), jsonScreen=cJSON_CreateObject());

        /* Iterate over conf list of screen */
        for (int n=0; n<conf_size; n++) {
            fieldName = sc->conf.at(n).fieldName;

            fieldType = sc->conf.at(n).fieldType;
            title = sc->conf.at(n).title;
            description = sc->conf.at(n).description;
            value = sc->conf.at(n).value;

            jsonConfArray = cJSON_AddArrayToObject(jsonScreen, fieldName.c_str());

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


// UWAGA! przepisac na queue albo semaphore
void screensManager_set_config(std::string screen_name, std::vector<std::string> conf_new) {
    ScreenConfig_t *sc;

    std::string name;

    int screens_size = screenList.size();
    int old_conf_size = 0;
    int new_conf_size = 0;
    int min_conf_size = 0;

    /* Find screen by name */
    for (int i=0; i<screens_size; i++) {
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
            return;
        }
    }

    ESP_LOGE(TAG, "Unable to set conf for screen %s", screen_name.c_str());
}

