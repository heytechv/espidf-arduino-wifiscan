#include "ScreenTime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <esp_log.h>
#include <esp_err.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/queue.h"

#include "display/display.h"
#include "esp_sntp.h"

#include <string>

static const char TAG[] = "ScreenTime";


std::string ScreenTime::getName() {
    return "Time";
}


std::vector<Screens::ConfigInput_t> ScreenTime::getDefaultConfig() {
    std::vector<ConfigInput_t> conf = {
        {
            Screens::GRAPHIC,
            "graphic",
            "Main graphic",
            "Graphic displayed next to time",
            "clock.anim"
        }
        // , {
        //     Screens::TEXT,
        //     "timezone",
        //     "Timezone",
        //     "Your city timezone",
        //     "CET-1CEST,M3.5.0,M10.5.0/3"  // Warsaw
        // }
    };

    return conf;
}


static uint8_t graphics_anim_frame = 0;
void ScreenTime::tick(Display *display, uint16_t ticks, std::vector<ConfigInput_t> conf) {


    if (ticks % 3 == 0) {
        ESP_LOGI(TAG, "%s = %s", conf.at(0).fieldName.c_str(), conf.at(0).value.c_str());
    }

    // char time_buff[64];

    time_t now;
    char strftime_buf[64];
    struct tm timeinfo;

    time(&now);

    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);  // Warsaw
    tzset();

    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);


    std::string strdatetime(strftime_buf);
    std::string sttime =  strdatetime.substr(11, 5);

    char *cstr = new char[sttime.length() + 1];
    strcpy(cstr, sttime.c_str());

    // ESP_LOGI(TAG, "The current date/time is: %s", cstr);

    /* Prepare display */
    display->clearBuffer();

    /* Graphics animation frame */
    // anim_frame ++;
    // if (anim_frame >= anim_pxci_clock_fc) anim_frame = 0;


    // /* Display graphics */
    // display->setCursor(0);
    // display->showPxci(anim_pxci_clock[anim_frame].graphics, (uint8_t (*)[3]) anim_pxci_clock[anim_frame].colormap, 0);   // "." used for non pointers, "->" for pointers

    display->setCursor(10);
    display->print(cstr);

    display->setCursor(0);
    helper_display_graphics(display,  conf.at(0).value.c_str(), ticks, 2, &graphics_anim_frame);

    /* Send buffer */
    display->sendBuffer();
}








