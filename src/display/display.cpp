#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "esp_log.h"

#include "font.h"
#include "display.h"
// #include "utils.h"
#include "ws2812b/ws2812b.h"




static const char TAG[] = "display";

WS2812B *ws2812b;



void Display::init() {
    display_cursor = 0;
    // clearBuffer();

    // ws2812b = new WS2812B();

    // ESP_LOGI("START", "START");
    // _DELAY_x40ns(50000000);
    // ESP_LOGI("STOP", "STOP");

    initSPIws2812();

}

void Display::setCursor(uint8_t c) {
    display_cursor = c;
}


uint8_t Display::getCursor() {
    return display_cursor;
}

void Display::setBrightness(uint8_t b) {
    display_brightess = b;
}

uint8_t Display::getBrightness() {
    return display_brightess;
}

void Display::clearBuffer() {
    memset(display_buffer, 0, BUFFER_LENGTH);  // copy 0 to display_buffer
}

void Display::sendBuffer() {
    // WS2812B_RESET();
    // for (int i=0; i<BUFFER_LENGTH; i+=3) {
    //     ws2812b->sendRGB(display_buffer[i], display_buffer[i+1], display_buffer[i+2]);
    // }
    // WS2812B_RESET();

    CRGB leds[256];
	for(int i = 0 ; i < 768 ; i+=3) {
		CRGB c;
        c.r = display_buffer[i];
        c.g = display_buffer[i+1];
        c.b = display_buffer[i+2];
		leds[i/3] = c;
	}

    fillBuffer((uint32_t*)&leds, 256);
	led_strip_update();



    

}

/**
 * @brief Displays single character on ws2812b matrix
 * 
 * @param c   character to display
 * @param rgb intensity (0-1)
 * 
 * @return sign_width: width of the character c (actual width of the sign encoded in font_8x8_default)
 */
uint8_t Display::printChar (char c) { return printfChar(c, 1, 1, 1); }
uint8_t Display::printfChar(char c, float r, float g, float b) {

    uint8_t bit_array[8], sign_width = 0;
    uint16_t buff_index = 0;

    for (uint8_t w = 0; w < 8; w++) {
        decToBin8(font_8x8_default[c - 32][w], bit_array);     // Z danego wiersza (ze tablicy znakow) rozkodowuje kolumne hex na bin
        
        for (uint8_t k = 0; k < 8; k++) {

            // Decode font -> ws2812b matrix
            buff_index = (uint16_t)font_to_buff_decode[w][k+display_cursor];

            // Load to buffer
            // jezeli przekracza zakres 0-32 to widocznie jest poza, nie wyswietlamy
            if (k+display_cursor < 32 && k+display_cursor >= 0) {
                display_buffer[buff_index  ] = bit_array[k]*display_brightess *r;
                display_buffer[buff_index+1] = bit_array[k]*display_brightess *g;
                display_buffer[buff_index+2] = bit_array[k]*display_brightess *b;
            }

            // Set cursor for next sign (find the max width of the current sign)
            if (k > sign_width && bit_array[k] > 0) {
                sign_width ++;
            }
        }
    }

    // Setting cursor for future sign
    // sign_width is now set at the end of the sign
    // but we want to +2 (+1 we are at the position after sign, +2 we are at the position after sign + one column space)
    sign_width += 2; display_cursor += sign_width;

    return sign_width;
}


/**
 * @brief Displays multiple characters on ws2812b matrix
 * 
 * @param c   character to display
 * @param rgb intensity (0-1)
 * 
 * @return sign_width: width of the character c (actual width of the sign encoded in font_8x8_default)
 */
uint8_t Display::print (char *text) { return printf(text, 1, 1, 1); }
uint8_t Display::printf(char *text, float r, float g, float b) {
   uint8_t text_width = 0;

    while (*text) {                                     // Until value pointed by var is not NULL
        text_width += printfChar(*(text++), r, g, b);   // print single char, then increment pointer to next char value in array
    }

    return text_width;
}

/**
 * @brief BITMAP 8x8
 * 
 * Bitmap pixel structure, START row (top left px first), next row, ...:
 *      last -> 0xB2, 0x22, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 *              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0xA4, 0xF4,
 *               ...
 *      next -> 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 *              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 *      START-> 0x11, 0xA1, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 *           -> 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0x23, 0xF3
 *   
 *   Starting with left to right [24 bytes, 8-byte * 3 -> (row data)*(rgb)]
 *
 *   So first row of our image (starting with left top pixel of our image):
 *      0x11, 0xA1, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0x23, 0xF3
 *      (1st px r,g,b)....(2nd px r,g,b)....
 *
 *  If we load it from the last index (191) to first (0) we will get mirrored image.
 *  To avoid that, just reverse painting every row with formual: 8 - column_number [8px = image width]
 *
 * @param graphics 
 * @param mirror_y true/false
 */
void Display::display_bmp(uint8_t graphics[192], uint8_t mirror_y) {

    uint16_t buff_index = 0;

    for (uint8_t w=0; w<8; w++) {
        for (uint8_t k=0; k<24; k+=3) {

            if (mirror_y) buff_index = font_to_buff_decode[w][k/3 + display_cursor];
            else          buff_index = font_to_buff_decode[w][8-k/3 + display_cursor];

            display_buffer[buff_index  ] = graphics[191-(w*24+k   )] * (float)(display_brightess/255.0f);
            display_buffer[buff_index+1] = graphics[191-(w*24+k +1)] * (float)(display_brightess/255.0f);
            display_buffer[buff_index+2] = graphics[191-(w*24+k +2)] * (float)(display_brightess/255.0f);
        }
    }

}

/**
 * @brief PXCI 8x8
 * For generating please use the script 'pxc_compress_bitmap.py'!
 *
 *  Graphics coded in:
 *      4bit4bit, 4bit4bit,...4bit4bit
 *      (leftTop(start)    ...rightBot(end))
 *
 *  4bit - coded color index (up to 16 colors)
 *  Colormap - contains indexes of colors
 *
 *  Compression:
 *      32*8 + 16*3*8
 *      graphics (32 bytes) + colormap (16 * 3[r,g,b] bytes)
 *
 *  Minimal compression (16 colors):
 *      1536 bytes -> 640 bytes (2.4 times smaller)
 *  Maximal compression (1 color):
 *      1536 bytes -> 304 bytes (5 times smaller)
 * 
 * @param graphics 
 * @param colormap 
 * @param shiftY 
 * @return uint8_t 
 */
uint8_t Display::showPxci(const uint8_t graphics[], const uint8_t colormap[][3], int8_t shiftY) {
    uint8_t pxH=0;
    uint8_t pxL=0;
    
    uint8_t k=0;
    uint16_t buff_index  = 0;

    for (uint8_t i=0; i<32; i++) {
        pxH = (graphics[i] >> 4);       // shift to get HIGH bits
        pxL = (graphics[i] & 0x0F);     // and mask to get LOW bits

        if (i%4 == 0) k = 0;                // each hex contains 8bits (2x4bits) so 8(byte) pixels (row) coded in 4 hexs

        if ((uint8_t)(i/4)+shiftY >= 8 || (uint8_t)(i/4)+shiftY < 0) continue;   // prevent too much and too small value

        buff_index = font_to_buff_decode[(uint8_t)(i/4)+shiftY][display_cursor + k++];  // decode first 4bits
        display_buffer[buff_index]   = colormap[pxH][0] * (float)(display_brightess/255.0f);    // decode colors using colomap
        display_buffer[buff_index+1] = colormap[pxH][1] * (float)(display_brightess/255.0f);
        display_buffer[buff_index+2] = colormap[pxH][2] * (float)(display_brightess/255.0f);

        buff_index = font_to_buff_decode[(uint8_t)(i/4)+shiftY][display_cursor + k++];  // decode next 4bits
        display_buffer[buff_index]   = colormap[pxL][0] * (float)(display_brightess/255.0f);    // decode colors using colomap
        display_buffer[buff_index+1] = colormap[pxL][1] * (float)(display_brightess/255.0f);
        display_buffer[buff_index+2] = colormap[pxL][2] * (float)(display_brightess/255.0f);
    }

    return 8;   // TODO width (for now const)
}

