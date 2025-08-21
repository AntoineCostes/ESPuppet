#include "LedStrip.h"

LedStrip::LedStrip(uint8_t pin, uint8_t numPixels, float brightness, neoPixelType type, float masterBrightness):
    Component("led_" + String(pin)),
    strip(numPixels, pin, type),
    numPixels(numPixels),
    pattern(RAINBOW),
    patternColor(4278387100),
    parameter(1.0f),
    lastLedChangeMs(millis()),
    increment(0)
{
    strip.begin();
    strip.setBrightness(255);
    this->brightness = min(1.0f, max(0.0f, brightness));
    this->masterBrightness = min(1.0f, max(0.0f, masterBrightness));
    clear(); // TODO clear leds after numPixels ?
}

void LedStrip::update()
{
    if (millis() > lastLedChangeMs + 50) // update every 50 ms (20 Hz)
    {
        increment++;
        int value = 0;
        switch (pattern)
        {
        case SOLID:
            break;
            
        case BLINK:
            value = 1000/parameter; // period = 1/freq
            if (millis()%value > value/2)  fill(patternColor);
            else clear();
            break;

        case OSCILLATOR:
            fill(patternColor, 0.5*(1+cos(2.0f*3.14f*parameter*millis()/1000.0)));
            lastLedChangeMs = millis();
            break;
            
        case CHASE:
            strip.clear();
            value = (int)(increment/10*parameter);
            for (int c=value%3; c<strip.numPixels(); c += 3)  strip.setPixelColor(c, patternColor); 
            strip.show();
        break;
        
        case RAINBOW:
            // long firstPixelHue = (increment*256)%5*65536;
            value = (int)(increment*256*parameter);
            strip.rainbow(value%(5*65536), 1, 255, masterBrightness*brightness*255, true);
            strip.show();
            break;
        
        case RANDOM:
            // long firstPixelHue = (increment*256)%5*65536;
            value = (int)(increment*256*parameter);
            strip.rainbow(value%(5*65536), 1, 255, 255, true);
            strip.show();
            break;
        }
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
    strip.fill(strip.Color(
        masterBrightness*brightness*pgm_read_byte(&gamma8[r]), 
        masterBrightness*brightness*pgm_read_byte(&gamma8[g]), 
        masterBrightness*brightness*pgm_read_byte(&gamma8[b])
        ));
    strip.show(); 
}

void LedStrip::setBrightness(float value)
{
    // TODO checkrange
    brightness = min(1.0f, max(0.0f, value));
}

void LedStrip::setPattern(LedPattern pattern, uint32_t patternColor, float parameter, float brightness)
{
    setBrightness(brightness);
    setPattern(pattern, patternColor, parameter);
}
    
void LedStrip::setPattern(LedPattern pattern, uint32_t patternColor, float parameter)
{
    increment = 0;
    switch (pattern)
    {
    case SOLID:
        fill(patternColor);
        break;
        
    case BLINK:
    if (parameter <= 0) parameter = 1.0f;
    break;
        
    case OSCILLATOR:
        parameter = abs(parameter);
    break;
        
    case CHASE:
    break;
        
    case RAINBOW:
    break;
    
    default:
        break;
    }
    this->pattern = pattern;
    this->patternColor = patternColor;
    this->parameter = parameter;
}