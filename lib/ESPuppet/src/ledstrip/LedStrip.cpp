#include "LedStrip.h"

LedStrip::LedStrip(uint8_t pin, uint8_t numPixels, float brightness, neoPixelType type, float masterBrightness):
    Component("led_" + String(pin)),
    strip(numPixels, pin, type),
    numPixels(numPixels),
    pattern(RAINBOW),
    patternColor(4278387100),
    parameter(1.0f),
    increment(0),
    refreshTimer(50, true), // 20Hz
    randomTimer(1000)
{
    strip.begin();
    strip.setBrightness(255);
    this->brightness = min(1.0f, max(0.0f, brightness));
    this->masterBrightness = min(1.0f, max(0.0f, masterBrightness));
    clear(); // TODO clear leds after numPixels ?

    refreshTimer.setCallback(std::bind(&LedStrip::refresh, this));
    refreshTimer.start();
}

void LedStrip::update()
{
    refreshTimer.update();
    randomTimer.update();
}

void LedStrip::refresh()
{
    increment++;
    int value = 0;
    switch (pattern)
    {
    case SOLID:
        break;
        
    case BLINK:
        value = 1000/ (parameter*10); // period = 1/freq
        if (millis()%value > value/2)  fill(patternColor);
        else clear();
        break;

    case OSCILLATOR:
        fill(patternColor, 0.5*(1+cos(2.0f*3.14f*parameter*millis()/1000.0)));
        break;
        
    case CHASE:
        strip.clear();
        value = (int)(increment/ (10*parameter*10));
        for (int c=value%3; c<strip.numPixels(); c += 3) setPixel(c, patternColor); 
        strip.show();
    break;
    
    case RAINBOW:
        value = (int)(increment*256*parameter*10); // firstPixelHue = (increment*256)%(5*65536);
        strip.rainbow(value%(5*65536), 1, 255, masterBrightness*brightness*255, true);
        strip.show();
        break;
    
    case RANDOM:
        if (!randomTimer.isRunning)
        {
            strip.clear();
            uint8_t red = (patternColor>>16) & 255;
            uint8_t green = (patternColor>>8) & 255;
            uint8_t blue = patternColor & 255;
            for (int i; i<strip.numPixels(); i++) 
            {
                float n = (float)inoise8(i, increment)/(float)255;
                setPixel(i, (uint8_t)(red * n), 
                            (uint8_t)(green * n), 
                            (uint8_t)(blue * n)); 
            }
            strip.show();
            
            randomTimer.set((int)(parameter*1000));
            randomTimer.start();
        }
        break;
        
    case BIRANDOM:
        strip.show();
        break;
        
    case GAUGE:
        strip.clear();
        value = (int) ( parameter*strip.numPixels() );
        for (int i; i<value; i++) setPixel(i, patternColor);
        strip.show();
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

void LedStrip::setPixel(uint8_t index, uint32_t color)
{
    uint8_t red = (color>>16) & 255;
    uint8_t green = (color>>8) & 255;
    uint8_t blue = color & 255;
    setPixel(index, red, green, blue);
}

void LedStrip::setPixel(uint8_t index, uint8_t r, uint8_t g, uint8_t b)
{
    strip.setPixelColor(index, strip.Color(
        masterBrightness*brightness*Adafruit_NeoPixel::gamma8(r), 
        masterBrightness*brightness*Adafruit_NeoPixel::gamma8(g), 
        masterBrightness*brightness*Adafruit_NeoPixel::gamma8(b)
        ));
}

void LedStrip::fill(uint8_t r, uint8_t g, uint8_t b)
{
    // TODO checkrange ?
    strip.fill(strip.Color(
        masterBrightness*brightness*Adafruit_NeoPixel::gamma8(r), 
        masterBrightness*brightness*Adafruit_NeoPixel::gamma8(g), 
        masterBrightness*brightness*Adafruit_NeoPixel::gamma8(b)
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
    parameter = min(1.0f, max(0.0f, parameter));
    increment = 0;
    randomTimer.stop();
    switch (pattern)
    {
    case SOLID:
        fill(patternColor);
        break;
        
    case BLINK:
    break;
        
    case OSCILLATOR:
    break;
        
    case CHASE:
    break;
        
    case RAINBOW:
    break;
        
    case RANDOM:
        randomTimer = Timer((int)(parameter*1000));
        randomTimer.start();
    break;
    
    default:
        break;
    }
    this->pattern = pattern;
    this->patternColor = patternColor;
    this->parameter = parameter;
}