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


void ScreenText::tick(Display *display, uint16_t ticks, std::vector<ConfigInput_t> conf) {
    /* Prepare display */
    display->clearBuffer();

    /* Graphics animation frame */
    // anim_frame ++;
    // if (anim_frame >= anim_pxci_loading_fc) anim_frame = 0;


    /* Display graphics */
    // display->showPxci(anim_pxci_loading[anim_frame].graphics, (uint8_t (*)[3]) anim_pxci_loading[anim_frame].colormap, 0);   // "." used for non pointers, "->" for pointers
    display->setCursor(10);
    display->print("Some text");

    /* Send buffer */
    portDISABLE_INTERRUPTS(); display->sendBuffer(); portENABLE_INTERRUPTS();
}

