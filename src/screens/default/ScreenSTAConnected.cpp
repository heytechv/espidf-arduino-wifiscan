#include "ScreenSTAConnected.h"

#include <tcpip_adapter.h>
#include <esp_wifi.h>
#include <esp_http_server.h>
#include "esp_http_client.h"
#include "esp_tls.h"
#include <string.h>
#include <string>


static const char TAG[] = "ScreenSTAConnected";

std::string ScreenSTAConnected::getName() {
    return "STAFailed";
}

std::vector<Screens::ConfigInput_t> ScreenSTAConnected::getDefaultConfig() {
    std::vector<ConfigInput_t> conf = {};
    return conf;
}

static uint8_t showIp = false;
static uint8_t wifi_disconnected_anim_frame = 0;
void ScreenSTAConnected::tick(Display *display, uint16_t ticks, std::vector<ConfigInput_t> conf) {


    if (ticks % 20 == 1) {
        showIp = true;
    }



    if (showIp == false) {
        display->clearBuffer();

        display->setCursor(0);
        display->print("PC");

        display->setCursor(11);
        helper_display_graphics(display, "arrow_right.bmp", ticks, 1, &wifi_disconnected_anim_frame);

        display->setCursor(23);
        helper_display_graphics(display, "wifi_connected.bmp", ticks, 1, &wifi_disconnected_anim_frame);

        /* Send buffer */
        display->sendBuffer();

    } else {
        tcpip_adapter_ip_info_t ipInfo; 
        char str[256];
                
        // IP address.
        tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);

        sprintf(str, IPSTR, IP2STR(&ipInfo.ip));


        std::string sRes(str);


        display_clearBuffer(display);
        display_setCursor(display, 0);
        display_text(display, sRes);
        display_sendBuffer(display);
    }

}

