#ifndef _EASY_BUTTON_H_
#define _EASY_BUTTON_H_

#include "stdlib.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/queue.h"


enum ButtonState {
    IDLE = 0,
    SINGLE_CLICK,
    DOUBLE_CLICK,
    MULTI_CLICK
};


// typedef struct Button {
//     uint8_t PIN;
//     uint32_t clickAmount = 0;
//     ButtonState state = IDLE;
//     unsigned long buttonTime = 0;
//     unsigned long lastTime = 0;
// } Button_t;


class EasyButton {
private:
    typedef void (*OnClickCallbackFunction)(ButtonState &);
    OnClickCallbackFunction listener = NULL;

    uint8_t PIN;

    unsigned long minClickTime = 50;

    uint8_t multiClickCounter = 0;
    unsigned long multiClickMaxTime = 500;

    uint8_t lastBtnState = 0;
    unsigned long lastBtnTime = 0;

public:
    void begin(uint8_t pin);
    void loop();
    void setOnClickListener(OnClickCallbackFunction f);
};



#endif
