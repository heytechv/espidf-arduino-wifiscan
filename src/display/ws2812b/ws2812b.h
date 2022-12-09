/*
 * ws2812b.h
 *
 * Created on Thu Dec 08 2022
 *
 * Copyright (c) 2022 Your Company
 * 
 * https://esp32.com/viewtopic.php?t=17900
 * https://github.com/8-DK/ESP32_SPI_WS2812_idf/blob/master/SPI_ws2812.h <<<<<<<<<<<< 
 */



#ifndef _WS2812B_H_
#define _WS2812B_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "driver/spi_master.h"
#include "driver/gpio.h"


/* ----------------------
 | Externs (ASM)
 ------------------------ */
extern "C" uint32_t mwait(uint32_t, uint32_t);

// extern "C" void _DELAY_x40ns(uint32_t);

extern "C" void     WS2812B_RESET();
extern "C" uint32_t WS2812B_SEND_BIT(uint32_t);



typedef struct CRGB {
    union {
        struct {
            union {
                uint8_t g;
                uint8_t green;
            };
            union {
                uint8_t r;
                uint8_t red;
            };
            union {
                uint8_t b;
                uint8_t blue;
            };
        };
        uint8_t raw[3];
        uint32_t num;
    };
} CRGB;


void initSPIws2812();
void fillCol(uint32_t col);
void fillBuffer(uint32_t* bufLed, int Count);
void led_strip_update();















/* ----------------------
 | WS2812B
 ------------------------ */

class WS2812B {

private:

    /**
     * @brief Send byte to ws2812b
     * 
     * @param _byte one byte to send
     */
    inline void sendByte(uint8_t _byte) {
        // TODO disable interrupts
        // TODO zoptymalizowac
        /* WS2812B first takes MOST significant bit (left one first) */
        for (int i = 7; i >= 0; i--) {
            WS2812B_SEND_BIT((_byte >> i));    // przesuwamy w prawo
        }
    }

public:

    /**
     * @brief Constructor
     */
    WS2812B();

    /**
     * @brief Send RGB bytes to ws2812b
     * 
     * @param r red
     * @param g green
     * @param b blue
     */
    void sendRGB(uint8_t r, uint8_t g, uint8_t b);

    /**
     * @brief Send RGBW bytes to ws2812b
     * 
     * @param r red
     * @param g green
     * @param b blue
     * @param w white
     */
    void sendRGBW(uint8_t r, uint8_t g, uint8_t b, uint8_t w);

};

#endif
