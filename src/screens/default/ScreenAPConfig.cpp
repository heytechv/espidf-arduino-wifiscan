#include "ScreenAPConfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <esp_log.h>
#include <esp_err.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/queue.h"

#include "display/display.h"

static const char TAG[] = "ScreenAPConfig";

int anim_frame2 = 0;



std::string ScreenAPConfig::getName() {
    return "APConfig";
}

std::vector<Screens::ConfigInput_t> ScreenAPConfig::getDefaultConfig() {
    std::vector<ConfigInput_t> conf = {};
    return conf;
}

static uint8_t download_blue_anim_frame = 0;
static uint8_t wifi_ap_anim_frame = 0;
void ScreenAPConfig::tick(Display *display, uint16_t ticks, std::vector<ConfigInput_t> conf) {
    /* Prepare display */
    display->clearBuffer();

    display->setCursor(0);
    display->print("PC");
    
    display->setCursor(11);
    helper_display_graphics(display, "download_blue.anim", ticks, 1, &download_blue_anim_frame);

    display->setCursor(23);
    helper_display_graphics(display, "wifi_ap.anim", ticks, 2, &wifi_ap_anim_frame);

    /* Send buffer */
    display->sendBuffer();
}

