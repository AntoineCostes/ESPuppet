#pragma once
#include "../common/Component.h"
#include "../util/Timer.h"

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
    bool analog;
    uint16_t periodMs;
    Timer blinkTimer;
    void timerEvent(const TimerEvent &e);
};