/*
 * Screens.h
 *
 * Created on Wed Nov 30 2022
 *
 * Copyright (c) 2022 Your Company
 */
#ifndef _SCREENS_H_
#define _SCREENS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

#include <esp_log.h>
#include <esp_err.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/queue.h"

#include "display/display.h"


class Screens {
public:

    enum ConfigFieldType {
        TEXT=0,
        FLOAT,
        INT,
        GRAPHIC
    };

    typedef struct ConfigInput_t {
        ConfigFieldType type;
        std::string name;
        std::string title;
        std::string description;
        std::string value;
        bool isNew;
    } ConfigInput_t;


    virtual std::string getName() { return "default"; }
    virtual std::vector<ConfigInput_t> getDefaultConfig();
    virtual void tick(Display *display, uint16_t ticks, std::vector<ConfigInput_t> conf) { }


    ConfigInput_t* getConfigByName(std::vector<ConfigInput_t> conf, std::string name) {
        for (int i=0; i<conf.size(); i++) {
            ConfigInput_t *c = &conf.at(0);

            if (c->name == name) {
                return c;
            }
        }
        return NULL;
    }



    void display_setCursor(Display *display, int16_t cursor) {
        display->setCursor(cursor);
    }

    int16_t display_getCursor(Display *display) {
        return display->getCursor();
    }

    uint8_t display_print(Display *display, std::string text) {
        char *ctext = new char[text.length() + 1];
        strcpy(ctext, text.c_str());

        uint8_t width = display->print(ctext);

        delete [] ctext;

        return width;
    }

    uint8_t display_getTextWidth(Display *display, std::string text) {
        char *ctext = new char[text.length() + 1];
        strcpy(ctext, text.c_str());

        uint8_t width = display->getWidthText(ctext);

        delete [] ctext;

        return width;
    }

    void display_clearBuffer(Display *display) {
        display->clearBuffer();
    }

    void display_sendBuffer(Display *display) {
        display->sendBuffer();
    }

    int16_t cursor_scroll = 0;
    void display_text(Display *display, std::string text) {
        uint8_t skip_scroll_amount = 12;  // todo do screen manager global config

        int8_t cursor_start = display_getCursor(display);
        uint8_t width = display_getTextWidth(display, text);
        int16_t cursor_new = cursor_start;

        /* Scrolling only for too long text */
        if (width >= 32 - cursor_start) {  // todo display width here (32px)
            cursor_scroll --;

            /* Let first word be longer (to make it easier to start reading for human) */
            if (cursor_scroll > -skip_scroll_amount) {
                cursor_new = cursor_start;

            /* After first word displayed for longer time, start scrolling */
            } else {
                cursor_new = cursor_scroll + skip_scroll_amount + cursor_start;
            }
        }

        /* Display */
        display_setCursor(display, cursor_new);
        display_print(display, text);

        /* Reset if scrolled everything */
        if (cursor_scroll < -width - cursor_start - skip_scroll_amount) {
            cursor_scroll = 0;
        }

    }


    void helper_display_graphics(Display *display, std::string input, uint16_t ticks, uint8_t ticks_divider, uint8_t *anim_frame) {
        if (ticks_divider == 0) ticks_divider = 1;

        /**
         * Check if animation. To do that we check if extension of the file is given.
         *   e.g. skull0.bmp = graphics
         *   e.g. skull.anim = animation
         **/
        bool isAnim = false;
        if (input.back() == 'm') {
            isAnim = true;
        }        

        /**
         * Get filename
         **/
        std::string filename = "";

        if (isAnim == true) {
            input = input.substr(0, input.size()-5);  // skull.anim -> skull
            filename = "/spiffs/" + input;            // /spiffs/skull
            filename += std::to_string(*anim_frame) + ".bmp";  // skull0.bmp, skull1.bmp ... skullX.bmp

            // ESP_LOGI("Screens.h", "anim: %s", filename.c_str());

        } else {
            filename = "/spiffs/" + input;            // /spiffs/skull0.bmp
            // ESP_LOGI("Screens.h", "img: %s", filename.c_str());
        } 

        /**
         * Try to open file.
         **/
        FILE *file = fopen(filename.c_str(), "rb");

        // For anim: if fil does not exist, we have gone too far with frames. Open first image.
        if (isAnim) {
            if(file == NULL) {
                *anim_frame = 0;  // reset anim counter
                filename = "/spiffs/" + input + "0.bmp";  // skull0.bmp
                file = fopen(filename.c_str(), "rb");
            }
        }

        /**
         * Increase anim counter
         */
        if (ticks % ticks_divider == 0) {
            (*anim_frame) ++;
        }
        // Overflow
        if (*anim_frame >= 250) {
            *anim_frame = 0;
        }

        /**
         * If file does not exists open "missing image" graphics.
         */
        if(file == NULL) {
            // todo missing icon
            ESP_LOGI("TAG", "Missing %s", filename.c_str());
            return;
        }

        /**
         * Load graphics
         **/
        uint8_t data_bmp[246];
        fread(data_bmp, sizeof(uint8_t), 246, file);
        fclose(file);

        // The actual size of minimal BMP file with 8x8 graphics should be 192 bytes -> (red + green + blue) * 8*8
        // The first 54 bytes are header we do not need
        display->display_bmp(data_bmp+54, false);
    }



};

#endif
