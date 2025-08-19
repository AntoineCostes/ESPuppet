#include "Output.h"

Output::Output(String name, int pin, byte start, bool inverse):Component(name), 
pin(pin), inverse(inverse), periodMs(0), blinkTimer(2000, true)
{
    pinMode(pin, OUTPUT);
    setPWM(start);

    blinkTimer.setCallback(std::bind(&Output::toggle, this));
}

void Output::setPWM(byte dutyCycle)
{
    if (dutyCycle == 0 || dutyCycle == 255) set(dutyCycle>0); // avoid using PWM channel for nothing
    else 
    {
        if (blinkTimer.isRunning)blinkTimer.stop(); 
        value = inverse?255-dutyCycle:dutyCycle;
        analogWrite(pin, value);
        analog = true;
    }
}

void Output::set(bool val)
{
    if (analog) // detach PWM
    {
        pinMode(pin, OUTPUT);
        analog = false;
    }   
    if (blinkTimer.isRunning) blinkTimer.stop(); 
    
    if (inverse) val = !val;
    value = val?255:0;
    digitalWrite(pin, val);
}

void Output::toggle()
{
    set (inverse? value != 0 :value == 0);
}

void Output::setTogglePeriodMs(uint16_t period)
{
    if (period == 0) blinkTimer.stop();
    else
    {
        blinkTimer.set(period/2, true);
        blinkTimer.start();
    }  
}

void Output::update()
{
    blinkTimer.update();
}