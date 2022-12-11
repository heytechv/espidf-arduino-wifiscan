#include "ScreenSTAConnecting.h"


static const char TAG[] = "ScreenSTAConnecting";

std::string ScreenSTAConnecting::getName() {
    return "STAFailed";
}

std::vector<Screens::ConfigInput_t> ScreenSTAConnecting::getDefaultConfig() {
    std::vector<ConfigInput_t> conf = {};
    return conf;
}

static uint8_t wifi_connecting_anim_frame = 0;
static uint8_t wifi_disconnected_anim_frame = 0;
void ScreenSTAConnecting::tick(Display *display, uint16_t ticks, std::vector<ConfigInput_t> conf) {
    display->clearBuffer();

    display->setCursor(0);
    display->print("PC");

    display->setCursor(11);
    helper_display_graphics(display, "loading_dots.anim", ticks, 2, &wifi_connecting_anim_frame);

    display->setCursor(23);
    helper_display_graphics(display, "wifi_ap2.bmp", ticks, 1, &wifi_disconnected_anim_frame);

    /* Send buffer */
    display->sendBuffer();
}

