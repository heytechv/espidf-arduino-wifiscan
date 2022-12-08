#include "ws2812b.h"
// #include <shared.h>

#include <esp_log.h>




WS2812B::WS2812B() {
    // WS2812B Pin is defined in "config.h" and "ws2812b_driver.S"
}

void WS2812B::sendRGB(uint8_t r, uint8_t g, uint8_t b) {
    /* WS2812B accepts in that order GRB */
    sendByte(g);
    sendByte(r);
    sendByte(b);
}

void WS2812B::sendRGBW(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
    /* WS2812B accepts in that order GRBW */
    sendByte(g);
    sendByte(r);
    sendByte(b);
    sendByte(w);
}

