#include "ScreenCrypto.h"

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

static const char TAG[] = "ScreenCrypto";

static std::string price = "-1";

static std::string crypto_response_buffer = "";

static esp_err_t weather_event_get_handler(esp_http_client_event_handle_t evt) {
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            crypto_response_buffer.append((char *)evt->data);
            break;
        default:
            break;
    }
    return ESP_OK;
}

static void get_weather(std::string symbol) {
    ESP_LOGI(TAG, "Getting crypto...");
    crypto_response_buffer = "";

    std::string weatherUrl = "https://api1.binance.com/api/v3/avgPrice?symbol=" + symbol;

    esp_http_client_config_t config_get = {
        .url = weatherUrl.c_str(),
        .cert_pem = NULL,
        .method = HTTP_METHOD_GET,
        .event_handler = weather_event_get_handler
    };
        
    esp_http_client_handle_t client = esp_http_client_init(&config_get);
    esp_http_client_perform(client);

    printf("OnCryptoData: %.*s\n", 6000, crypto_response_buffer.c_str());

    /* Parse data */
    cJSON *root2 = cJSON_Parse(crypto_response_buffer.c_str());
    if (root2 == NULL) {
        price = "-1";
        return;
    }
    
    cJSON *p = cJSON_GetObjectItem(root2, "price");
    if (p == NULL) {
        price = "-1";
        return;
    }

    price = p->valuestring;

    ESP_LOGI(TAG, "data %s", price.c_str());

    /* Cleaning up */
    esp_http_client_cleanup(client);
    cJSON_Delete(root2);
}


std::string ScreenCrypto::getName() {
    return "Crypto";
}

std::vector<Screens::ConfigInput_t> ScreenCrypto::getDefaultConfig() {
    std::vector<ConfigInput_t> conf = {
        {
            Screens::TEXT,
            "symbol",
            "Symbol",
            "Symbol of cryptocurrency e.g. BTCUSDT.",
            "BTCUSDT"
        }
    };
    return conf;
}


static std::string symbol = "BTCUSDT";
static bool isSymbolChanged = true;

static uint8_t graphic_anim = 0;
void ScreenCrypto::tick(Display *display, uint16_t ticks, std::vector<ConfigInput_t> conf) {

    std::string s = conf.at(0).value;

    if (symbol != s) {
        isSymbolChanged = true;
        symbol = s;
        ESP_LOGI(TAG, "Symbol changed. Updating...");
    }

    /* Get weather on start and every 1 hour */
    if (ticks % 36000 == 0 || price == "-1" || isSymbolChanged) {
        isSymbolChanged = false;
        price = "0";  // zeby na pewno nie spamic
        get_weather(s);
    }

    /* Preparing price string */
    std::string priceString = price;
    priceString = priceString.substr(0, priceString.find(".")+2);
    priceString = s + ": " + priceString;

    /* Display temp */
    display_clearBuffer(display);
    display_setCursor(display, 10);
    display_text(display, priceString);

    /* Display graphic */
    display_setCursor(display, 0);
    helper_display_graphics(display, "tick.bmp", ticks, 4, &graphic_anim);

    /* Send buffer */
    display_sendBuffer(display);
}









