#include "ScreenLoading.h"

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <string>

#include <esp_log.h>
#include <esp_err.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/queue.h"

#include "display/display.h"
#include "esp_spiffs.h"

static const char TAG[] = "ScreenLoading";


std::string ScreenLoading::getName() {
    return "Loading";
}

std::vector<Screens::ConfigInput_t> ScreenLoading::getDefaultConfig() {
    std::vector<ConfigInput_t> conf = {};
    return conf;
}


static uint8_t graphics_anim_frame = 0;
void ScreenLoading::tick(Display *display, uint16_t ticks, std::vector<ConfigInput_t> conf) {
    /* Prepare display */
    display->clearBuffer();
    display->setCursor(11);
    

    helper_display_graphics(display, "loading.anim", ticks, 2, &graphics_anim_frame);

    /* Send buffer */
    display->sendBuffer();
}

// uint8_t anim_frame_rob = 0;
// void helper_display_graphics(Display *display, std::string input, uint8_t time_divider, uint16_t ticks) {

//     /**
//      * Check if animation. To do that we check if extension of the file is given.
//      *   e.g. skull_0.bmp = graphics
//      *   e.g. skull_      = animation
//      **/
//     bool isAnim = true;
//     if (input.back() == 'p') {
//         isAnim = false;
//     }

//     /**
//      * Get filename
//      **/
//     std::string filename = "";
//     if (isAnim == false) {
//         filename = "/spiffs/" + input;  // skull_0.bmp
//     } else {
//         filename = "/spiffs/" + input;  // skull_
//         filename += std::to_string(anim_frame_rob) + ".bmp";  // skull_0.bmp, skull_1.bmp ... skull_X.bmp
//     }

//     /**
//      * Try to open file.
//      **/
//     FILE *file = fopen(filename.c_str(), "rb");

//     // For anim: if fil does not exist, we have gone too far with frames. Open first image.
//     if (isAnim) {
//         if(file == NULL) {
//             anim_frame_rob = 0;  // reset anim counter
//             filename = "/spiffs/" + input + "0.bmp";  // skull_0.bmp
//             file = fopen(filename.c_str(), "rb");
//         }
//     }

//     /**
//      * Increase anim counter
//      */
//     if (ticks % time_divider == 0) {
//         anim_frame_rob ++;
//     }
//     // Overflow
//     if (anim_frame_rob > 255) {
//         anim_frame_rob = 0;
//     }

//     /**
//      * If file does not exists open "missing image" graphics.
//      */
//     if(file == NULL) {
//         // todo missing icon
//         ESP_LOGI("TAG", "Missing %s", filename.c_str());
//         return;
//     }

//     /**
//      * Load graphics
//      **/
//     uint8_t data_bmp[246];
//     fread(data_bmp, sizeof(uint8_t), 246, file);
//     fclose(file);

//     // The actual size of minimal BMP file with 8x8 graphics should be 192 bytes -> (red + green + blue) * 8*8
//     // The first 54 bytes are header we do not need
//     display->display_bmp(data_bmp+54, false);
// }








