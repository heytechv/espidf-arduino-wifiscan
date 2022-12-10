#include "ScreenSTAConnected.h"


static const char TAG[] = "ScreenSTAConnected";

std::string ScreenSTAConnected::getName() {
    return "STAFailed";
}

std::vector<Screens::ConfigInput_t> ScreenSTAConnected::getDefaultConfig() {
    std::vector<ConfigInput_t> conf = {};
    return conf;
}

static uint8_t wifi_disconnected_anim_frame = 0;
void ScreenSTAConnected::tick(Display *display, uint16_t ticks, std::vector<ConfigInput_t> conf) {
    display->clearBuffer();

    display->setCursor(0);
    display->print("PC");

    display->setCursor(11);
    helper_display_graphics(display, "arrow_right.bmp", ticks, 1, &wifi_disconnected_anim_frame);

    display->setCursor(23);
    helper_display_graphics(display, "wifi_connected.bmp", ticks, 1, &wifi_disconnected_anim_frame);

    /* Send buffer */
    display->sendBuffer();
}

