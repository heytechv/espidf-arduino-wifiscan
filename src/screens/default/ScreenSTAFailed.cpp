#include "ScreenSTAFailed.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <esp_log.h>
#include <esp_err.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/queue.h"

#include "display/display.h"


static const char TAG[] = "ScreenSTAFailed";


std::string ScreenSTAFailed::getName() {
    return "STAFailed";
}

std::vector<Screens::ConfigInput_t> ScreenSTAFailed::getDefaultConfig() {
    std::vector<ConfigInput_t> conf = {};
    return conf;
}

static uint8_t wifi_disconnected_anim_frame = 0;
void ScreenSTAFailed::tick(Display *display, uint16_t ticks, std::vector<ConfigInput_t> conf) {
    /* Prepare display */
    display->clearBuffer();

    display->setCursor(0);
    display->print("PC");

    display->setCursor(7);
    helper_display_graphics(display, "connection_broken_part0.bmp", ticks, 1, &wifi_disconnected_anim_frame);
    display->setCursor(15);
    helper_display_graphics(display, "connection_broken_part1.bmp", ticks, 1, &wifi_disconnected_anim_frame);

    display->setCursor(23);
    helper_display_graphics(display, "wifi_disconnected.bmp", ticks, 1, &wifi_disconnected_anim_frame);

    /* Send buffer */
    display->sendBuffer();
}

