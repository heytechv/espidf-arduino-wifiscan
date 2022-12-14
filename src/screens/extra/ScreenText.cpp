#include "ScreenText.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <esp_log.h>
#include <esp_err.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/queue.h"

#include "display/display.h"

static const char TAG[] = "ScreenText";


std::string ScreenText::getName() {
    return "Text";
}

std::vector<Screens::ConfigInput_t> ScreenText::getDefaultConfig() {
    std::vector<ConfigInput_t> conf = {
        {
            Screens::GRAPHIC,
            "icon",
            "Graphic",
            "Grtaphic displayed",
            "tick.bmp"
        },
        {
            Screens::TEXT,
            "text",
            "Text",
            "Text displayed",
            "Text"
        }
    };
    return conf;
}

static int16_t scroll_cursor = 0;
static uint8_t graphic_anim = 0;
void ScreenText::tick(Display *display, uint16_t ticks, std::vector<ConfigInput_t> conf) {
    std::string g = conf.at(0).value;
    std::string t = conf.at(1).value;

    display_clearBuffer(display);
    display_setCursor(display, 10);
    display_text(display, t);

    display_setCursor(display, 0);
    helper_display_graphics(display, g, ticks, 3, &graphic_anim);

    /* Send buffer */
    display_sendBuffer(display);
}





