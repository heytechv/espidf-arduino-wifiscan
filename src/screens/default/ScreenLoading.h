/*
 * ScreenSetup.h
 *
 * Created on Wed Nov 30 2022
 *
 * Copyright (c) 2022 Your Company
 */
#ifndef _SCREEN_LOADING_H_
#define _SCREEN_LOADING_H_

#include <stdlib.h>
#include "screens/Screens.h"
#include "display/display.h"


class ScreenLoading : public Screens {
public:

    virtual void tick(Display *display, uint16_t ticks, std::vector<ConfigInput_t> conf) override;


};



#endif

