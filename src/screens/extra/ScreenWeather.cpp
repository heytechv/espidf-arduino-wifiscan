#include "ScreenWeather.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <esp_log.h>
#include <esp_err.h>

#include "esp_http_client.h"
#include "esp_tls.h"

#include "cJSON.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/queue.h"

#include "display/display.h"

static const char TAG[] = "ScreenWeather";

static double weather_temp = 0;
static int weather_code = -1;

static std::string weather_response_buffer = "";

static esp_err_t weather_event_get_handler(esp_http_client_event_handle_t evt) {
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            weather_response_buffer.append((char *)evt->data);
            break;
        default:
            break;
    }
    return ESP_OK;
}

static void get_weather(std::string lat, std::string lon) {
    ESP_LOGI(TAG, "Getting weather...");
    weather_response_buffer = "";

    std::string weatherUrl = "http://api.open-meteo.com/v1/forecast?latitude="+lat+"&longitude="+lon+"&daily=weathercode,apparent_temperature_max&timezone=Europe%2FLondon";

    esp_http_client_config_t config_get = {
        .url = weatherUrl.c_str(),
        .cert_pem = NULL,
        .method = HTTP_METHOD_GET,
        .event_handler = weather_event_get_handler
    };
        
    esp_http_client_handle_t client = esp_http_client_init(&config_get);
    esp_http_client_perform(client);

    // printf("OnWeatherData: %.*s\n", 6000, weather_response_buffer.c_str());

    /* Parse data */
    cJSON *root2 = cJSON_Parse(weather_response_buffer.c_str());

    cJSON *daily = cJSON_GetObjectItem(root2, "daily");
    cJSON *apparent_temperature_max = cJSON_GetObjectItem(daily, "apparent_temperature_max");
    cJSON *weathercode = cJSON_GetObjectItem(daily, "weathercode");

    weather_temp = cJSON_GetArrayItem(apparent_temperature_max, 0)->valuedouble;
    weather_code = cJSON_GetArrayItem(weathercode, 0)->valueint;

    /* Cleaning up */
    esp_http_client_cleanup(client);
}


std::string ScreenWeather::getName() {
    return "Weather";
}

std::vector<Screens::ConfigInput_t> ScreenWeather::getDefaultConfig() {
    std::vector<ConfigInput_t> conf = {
        {
            Screens::FLOAT,
            "lat",
            "Latitude",
            "Latitude of your position",
            "50.28"
        },
        {
            Screens::FLOAT,
            "lon",
            "Longitude",
            "Longitude of your position",
            "18.81"
        }
    };
    return conf;
}

static int16_t scroll_cursor = 0;
static uint8_t graphic_anim = 0;
void ScreenWeather::tick(Display *display, uint16_t ticks, std::vector<ConfigInput_t> conf) {

    /* Get weather on start and every 1 hour */
    if (ticks % 36000 == 0 || weather_code == -1) {

        weather_code = -2;  // zeby na pewno nie spamic

        std::string lat = conf.at(0).value;
        std::string lon = conf.at(1).value;

        get_weather(lat, lon);
    }

    /* Convert temp to string */
    std::string temp_text = std::to_string(weather_temp);
    temp_text = temp_text.substr(0, temp_text.find(".")+2);
    temp_text += " C";

    /* Get graphic according to weathercode */
    std::string icon_name = "weather_partly_cloudly.bmp";
    if (weather_code == 0) {
        icon_name = "weather_sun.bmp";
    } else if (weather_code > 0 && weather_code <= 48) {
        icon_name = "weather_partly_cloudly.bmp";
    } else if (weather_code > 48 && weather_code <= 67) {
        icon_name = "weather_rain.anim";
    } else if (weather_code > 67 && weather_code <= 77) {
        icon_name = "weather_snow.anim";
    } else if (weather_code > 77) {
        icon_name = "weather_rain.anim";
    }

    // ESP_LOGI(TAG, "weathercode: %d, weathericon: %s", weather_code, icon_name.c_str());

    /* Display temp */
    display_clearBuffer(display);
    display_setCursor(display, 10);
    display_print(display, temp_text);

    /* Display graphic */
    display_setCursor(display, 0);
    helper_display_graphics(display, icon_name, ticks, 4, &graphic_anim);

    /* Send buffer */
    display_sendBuffer(display);
}









