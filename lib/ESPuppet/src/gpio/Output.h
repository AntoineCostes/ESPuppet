#pragma once
#include "../common/Component.h"

class Output : public Component
{
public:
    Output(String name, int pin, byte start, bool inverse);
    void update() override;

    void setPWM(byte dutyCycle);
    void set(bool val);
    void toggle();
    void setTogglePeriodMs(uint16_t period);

protected:
    int pin;
    byte value;
    bool inverse;
    uint16_t periodMs;
};