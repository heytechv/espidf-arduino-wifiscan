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
    std::vector<ConfigInput_t> conf = {};
    return conf;
}




// void helper_display_text(Display *display, std::string input) {
    
//     uint8_t cursor_start = display->getCursor();


//     display->setCursor(cursor_start + scroll_cursor);

//     char *cstr = new char[input.length() + 1];
//     strcpy(cstr, input.c_str());
//     uint8_t width = display->print(cstr);
//     delete [] cstr;


//     if (width >= 32 - cursor_start) {

//         scroll_cursor --;

//         if (scroll_cursor > -32) {

//             display->clearBuffer();

//             display->setCursor(cursor_start);

//             char *cstr = new char[input.length() + 1];
//             strcpy(cstr, input.c_str());
//             uint8_t width = display->print(cstr);
//             delete [] cstr;

//         } else {
//             int16_t corrected_cursor = scroll_cursor + 32;

//             display->clearBuffer();

//             display->setCursor(cursor_start + corrected_cursor);

//             char *cstr = new char[input.length() + 1];
//             strcpy(cstr, input.c_str());
//             uint8_t width = display->print(cstr);
//             delete [] cstr;

//         }

        

//         // reset if scrolled everything
//         if (scroll_cursor < -width - cursor_start - 32) {
//             scroll_cursor = 0;
//         }
//     }

    

//     ESP_LOGI(TAG, "%d", scroll_cursor);


// }




static int16_t scroll_cursor = 0;
static uint8_t graphic_anim = 0;
void ScreenText::tick(Display *display, uint16_t ticks, std::vector<ConfigInput_t> conf) {
    /* Prepare display */
    // display->clearBuffer();


    /* Graphics animation frame */
    // anim_frame ++;
    // if (anim_frame >= anim_pxci_loading_fc) anim_frame = 0;


    /* Display graphics */
    // display->showPxci(anim_pxci_loading[anim_frame].graphics, (uint8_t (*)[3]) anim_pxci_loading[anim_frame].colormap, 0);   // "." used for non pointers, "->" for pointers
    // display->setCursor(10);

    display_clearBuffer(display);
    display_setCursor(display, 10);
    display_text(display, "Some text for scrolling XDDD", &scroll_cursor);

    display_setCursor(display, 0);
    helper_display_graphics(display, "clock0.bmp", ticks, 1, &graphic_anim);

    /* Send buffer */
    // display->sendBuffer();
    display_sendBuffer(display);
}





