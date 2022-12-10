#ifndef _SCREEN_STA_CONNECTED_H_
#define _SCREEN_STA_CONNECTED_H_

#include "screens/Screens.h"


class ScreenSTAConnected : public Screens {
public:

    virtual std::string getName() override;
    virtual std::vector<ConfigInput_t> getDefaultConfig() override;
    virtual void tick(Display *display, uint16_t ticks, std::vector<ConfigInput_t> conf) override;

};

#endif
