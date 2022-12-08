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


void ScreenAPConfig::tick(Display *display, uint16_t ticks, std::vector<ConfigInput_t> conf) {
    /* Prepare display */
    display->clearBuffer();

    /* Graphics animation frame */
    if (ticks%4 == 0) anim_frame ++;
    if (ticks%2 == 0) anim_frame2 ++;


    // if (anim_frame >= anim_pxci_wifi_ap_fc) anim_frame = 0;
    // if (anim_frame2 >= anim_pxci_arrow_right_fc) anim_frame2 = 0;


    // /* Display graphics */
    // display->setCursor(0);
    // display->showPxci(anim_pxci_wifi_ap[anim_frame].graphics, (uint8_t (*)[3]) anim_pxci_wifi_ap[anim_frame].colormap, 0);   // "." used for non pointers, "->" for pointers

    // display->setCursor(12);
    // display->showPxci(anim_pxci_arrow_right[anim_frame2].graphics, (uint8_t (*)[3]) anim_pxci_arrow_right[anim_frame2].colormap, 0);   // "." used for non pointers, "->" for pointers


    /* Send buffer */
    portDISABLE_INTERRUPTS(); display->sendBuffer(); portENABLE_INTERRUPTS();
}

