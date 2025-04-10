#include "LedStrip.h"

LedStrip::LedStrip(uint8_t pin, uint8_t numPixels, float brightness, neoPixelType type):
    Component("led_" + String(pin)),
    strip(numPixels, pin, type),
    numPixels(numPixels),
    pattern(STEADY),
    frequency(1.0f),
    lastLedChangeMs(millis())
{
    strip.begin();
    strip.setBrightness(255);
    this->brightness = min(1.0f, max(0.0f, brightness));
    clear(); // TODO clear leds after numPixels ?
}

void LedStrip::update()
{
    switch (pattern)
    {
    case STEADY:
        break;

    case OSCILLATE:
        if (millis() > lastLedChangeMs + 50) // update every 50 ms (20 Hz)
        {
            float wave = 0.5*(1+cos(2.0f*3.14f*frequency*millis()/1000.0));
            fill(patternColor, wave);
            lastLedChangeMs = millis();
        }
        
        break;
        
    case BLINK:
        long duration = 1000/frequency; // FIXME handle freq = 0
        if (millis()%(duration) > duration/2)  fill(patternColor);
        else clear();
        break;
    }
}

void LedStrip::clear()
{
    strip.clear();
    strip.show();
}

void LedStrip::fill(uint32_t color)
{
    uint8_t red = (color>>16) & 255;
    uint8_t green = (color>>8) & 255;
    uint8_t blue = color & 255;
    fill(red, green, blue);
}

void LedStrip::fill(uint32_t color, float multiplier)
{
    uint8_t red = (color>>16) & 255;
    uint8_t green = (color>>8) & 255;
    uint8_t blue = color & 255;
    fill(multiplier*red, multiplier*green, multiplier*blue);
}

void LedStrip::fill(uint8_t r, uint8_t g, uint8_t b)
{
    // TODO checkrange ?
    strip.fill(strip.Color(brightness*r, brightness*g, brightness*b));
    strip.show(); 
}

void LedStrip::setBrightness(float value)
{
    // TODO checkrange
    brightness = min(1.0f, max(0.0f, value));
}

void LedStrip::setSolid(uint8_t r, uint8_t g, uint8_t b)
{
    pattern = LedPattern::STEADY;
    fill(r, g, b);
}

void LedStrip::setWave(uint8_t r, uint8_t g, uint8_t b, float frequency)
{
    pattern = LedPattern::OSCILLATE;
    this->frequency = abs(frequency);
    patternColor = strip.Color(r, g, b);
}

void LedStrip::setBlink(uint8_t r, uint8_t g, uint8_t b, float frequency)
{
    pattern = LedPattern::BLINK;
    this->frequency = frequency>0?frequency:1.0f;
    patternColor = strip.Color(r, g, b);
}

void LedStrip::notifyWave(uint8_t r, uint8_t g, uint8_t b, float frequency)
{

}

void LedStrip::notifyBlink(uint8_t r, uint8_t g, uint8_t b, float frequency)
{

}