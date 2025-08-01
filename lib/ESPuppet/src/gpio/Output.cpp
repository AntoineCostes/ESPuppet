#include "Output.h"

Output::Output(String name, int pin, byte start, bool inverse):Component(name), pin(pin), inverse(inverse), periodMs(0)
{
    pinMode(pin, OUTPUT);
    setPWM(start);
}

void Output::setPWM(byte dutyCycle)
{
    value = inverse?255-dutyCycle:dutyCycle;
    analogWrite(pin, value);
}

void Output::set(bool val)
{
    setPWM(val?255:0);
}

void Output::toggle()
{
    set (value == 0);
}

void Output::setTogglePeriodMs(uint16_t period)
{
    periodMs = period;
}

void Output::update()
{
    if (periodMs > 0)
        set( millis()%periodMs > periodMs / 2);
}