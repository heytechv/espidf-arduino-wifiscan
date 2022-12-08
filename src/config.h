#ifndef _CONFIG_H_
#define _CONFIG_H_

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



#endif
