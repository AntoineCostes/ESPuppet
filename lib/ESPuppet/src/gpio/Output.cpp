#include "Output.h"

Output::Output(String name, int pin, byte start, bool inverse):Component(name), pin(pin), inverse(inverse), periodMs(0), timer(2000, true)
{
    pinMode(pin, OUTPUT);
    setPWM(start);
    
    timer.addListener(std::bind(&Output::timerEvent, this, std::placeholders::_1));
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
    if (period == 0) timer.stop();
    else
    {
        timer.set(period, true);
        timer.start();
    }  
}

void Output::timerEvent(const TimerEvent &e)
{
    toggle();
    Serial.println(millis());
}

void Output::update()
{
    timer.update();
}