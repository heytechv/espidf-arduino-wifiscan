/*
 * Screens.h
 *
 * Created on Wed Nov 30 2022
 *
 * Copyright (c) 2022 Your Company
 */
#ifndef _SCREENS_H_
#define _SCREENS_H_

#include <stdlib.h>
#include "display/display.h"
#include <string.h>
#include <string>
 #include <vector>

class Screens {
public:

    enum ConfigFieldType {
        TEXT=0,
        GRAPHIC
    };

    typedef struct ConfigInput_t {
        ConfigFieldType fieldType;
        std::string fieldName;
        std::string title;
        std::string description;
        std::string value;
    } ConfigInput_t;


    /* Graphics - frame by frame animation */
    uint8_t anim_frame = 0;  // current animation frame


    virtual std::string getName() { return "default"; }
    virtual std::vector<ConfigInput_t> getDefaultConfig();

    virtual void tick(Display *display, uint16_t ticks, std::vector<ConfigInput_t> conf) { }

};

#endif
