#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <stdlib.h>

#define WS2812B_PIN 27

#define MATRIX_HEIGHT   8
#define MATRIX_WIDTH    32
#define MATRIX_LEDS     (MATRIX_HEIGHT*MATRIX_WIDTH)
#define BUFFER_LENGTH   (MATRIX_LEDS*3)     // RGB

#define DISPLAY_BTIGHTNESS 32   // Brightness (1-255), Default: 32 (please consider using better power source if u want to increase this nubmer greatly)

/**
 * @brief One frame of image (type)
 * 
 */
typedef struct GraphicsFrame_t {
    const uint8_t *graphics;
    const uint8_t *colormap;
} GraphicsFrame_t;


class Display {
private:

    const uint16_t font_to_buff_decode[8][32] = { // dekoduje uklad czcionki czy grafiki (wierszami) na uklad WS2812B matrix (gdzie idzie dziwnie wezykiem)
        {0, 45, 48, 93, 96, 141, 144, 189, 192, 237, 240, 285, 288, 333, 336, 381, 384, 429, 432, 477, 480, 525, 528, 573, 576, 621, 624, 669, 672, 717, 720, 765},
        {3, 42, 51, 90, 99, 138, 147, 186, 195, 234, 243, 282, 291, 330, 339, 378, 387, 426, 435, 474, 483, 522, 531, 570, 579, 618, 627, 666, 675, 714, 723, 762},
        {6, 39, 54, 87, 102, 135, 150, 183, 198, 231, 246, 279, 294, 327, 342, 375, 390, 423, 438, 471, 486, 519, 534, 567, 582, 615, 630, 663, 678, 711, 726, 759},
        {9, 36, 57, 84, 105, 132, 153, 180, 201, 228, 249, 276, 297, 324, 345, 372, 393, 420, 441, 468, 489, 516, 537, 564, 585, 612, 633, 660, 681, 708, 729, 756},
        {12, 33, 60, 81, 108, 129, 156, 177, 204, 225, 252, 273, 300, 321, 348, 369, 396, 417, 444, 465, 492, 513, 540, 561, 588, 609, 636, 657, 684, 705, 732, 753},
        {15, 30, 63, 78, 111, 126, 159, 174, 207, 222, 255, 270, 303, 318, 351, 366, 399, 414, 447, 462, 495, 510, 543, 558, 591, 606, 639, 654, 687, 702, 735, 750},
        {18, 27, 66, 75, 114, 123, 162, 171, 210, 219, 258, 267, 306, 315, 354, 363, 402, 411, 450, 459, 498, 507, 546, 555, 594, 603, 642, 651, 690, 699, 738, 747},
        {21, 24, 69, 72, 117, 120, 165, 168, 213, 216, 261, 264, 309, 312, 357, 360, 405, 408, 453, 456, 501, 504, 549, 552, 597, 600, 645, 648, 693, 696, 741, 744}
    };

    uint8_t display_buffer[BUFFER_LENGTH] = {};      // Bufor ramki, https://pl.wikipedia.org/wiki/Bufor_ramki, TODO define 768
    uint8_t display_cursor = 0;
    uint8_t display_brightess = DISPLAY_BTIGHTNESS;  // Brightness (1-255), Default: 32 (please consider using better power source if u want to increase this nubmer greatly)

    /**
     * Decimal to binary conversion
     */
    void decToBin8(uint8_t dec_num, uint8_t *bit_array) {
       /* Reszta z dzielenia i dzielimy */
        for (uint8_t i = 0; i < 8; i++) {
            bit_array[8-i-1] = dec_num % 2;
            dec_num /= 2;
        }
    }

public:

    /**
     * @brief Display init (clear buffer, reset cursor)
     * 
     */
    void init();

    /**
     * @brief Set display cursor
     * 
     * @param c new cursor position
     */
    void setCursor(uint8_t c);

    /**
     * @brief Get display cursor
     * @return cursor position
     */
    uint8_t getCursor();

    /**
     * @brief Set display brightness
     * 
     * @param b new brightness
     */
    void setBrightness(uint8_t b);

    /**
     * @brief Get display brightness
     * @return brightness
     */
    uint8_t getBrightness();

    /**
     * @brief Clear buffer
     * 
     */
    void clearBuffer();

    /**
     * @brief Send Buffer
     * 
     */
    void sendBuffer();

    /**
     * @brief Populate buffer with char (font data) 
     * Decodes char using font array
     * Manages cursor and prepare it for next font character
     * 
     * @param c character
     * @param rgb intensity (0-1)
     * @return uint8_t width of the character (according to font array)
     */
    uint8_t printfChar(char c, float r, float g, float b);
    uint8_t printChar (char c);

    /**
     * @brief Populate buffer with mutiple characters (font data)
     * Calls display_printf_char() multiple times
     * Manages cursor
     * 
     * @param text text
     * @param rgb intensity (0-1)
     * @return uint8_t width of the text (according to font array)
     */
    uint8_t printf(char *text, float r, float g, float b);
    uint8_t print (char *text);

    /**
     * @brief Populate buffer with bitmap
     * BMP 8x8 TODO any size
     * 
     * @param postac 
     * @param frame 
     */
    void display_bmp(uint8_t graphics[192], uint8_t mirror_y);

    /**
     * @brief Populate buffer with custom image format pxci (pixel clock image)
     * 
     * PXCI 8x8 // TODO any size
     * For generating please use the script 'pxc_compress_bitmap.py'!
     * 
     * Graphics coded in:
     *  4bit4bit, 4bit4bit,...4bit4bit
     *  (leftTop           ...rightBot)
     *
     * 4bit - coded color index (up to 16 colors)
     *
     * Colormap - contains indexes of colors
     * 
     * Compression:
     *  32*8 + 16*3*8
     *      graphics (32 bytes) + colormap (16 * 3[r,g,b] bytes)
     * 
     * Minimal compression (16 colors):
     *  1536 bytes -> 640 bytes (2.4 times smaller)
     * 
     * Maximal compression (1 color):
     *  1536 bytes -> 304 bytes (5 times smaller)
     * 
     * @param graphics 8x8 graphics (max 16 colors)
     * @param colormap up to 16 colors colormap
     * @param shiftY shift image vertically
     * @return uint8_t width of an image (todo)
     */
    uint8_t showPxci(const uint8_t graphics[32], const uint8_t colormap[][3], int8_t shiftY);

};

#endif
