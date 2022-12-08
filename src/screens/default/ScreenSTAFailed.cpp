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




void ScreenSTAFailed::tick(Display *display, uint16_t ticks, std::vector<ConfigInput_t> conf) {
    /* Prepare display */
    display->clearBuffer();

    /* Graphics animation frame */
    // if (ticks%4 == 0) anim_frame ++;


    // if (anim_frame >= anim_pxci_wifi_ap_fc) anim_frame = 0;


    /* Display graphics */
    // display->setCursor(0);
    // display->showPxci(anim_pxci_wifi_ap[anim_frame].graphics, (uint8_t (*)[3]) anim_pxci_wifi_ap[anim_frame].colormap, 0);   // "." used for non pointers, "->" for pointers

    // display->setCursor(12);
    // display->showPxci(i_pxci_xsign_gp, (uint8_t (*)[3]) i_pxci_xsign_cm, 0);   // "." used for non pointers, "->" for pointers


    /* Send buffer */
    portDISABLE_INTERRUPTS(); display->sendBuffer(); portENABLE_INTERRUPTS();
}

