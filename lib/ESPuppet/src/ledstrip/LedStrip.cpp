#include "LedStrip.h"

LedStrip::LedStrip(uint8_t pin, uint8_t numPixels, float brightness, neoPixelType type, float masterBrightness):
    Component("led_" + String(pin)),
    strip(numPixels, pin, type),
    numPixels(numPixels),
    pattern(RAINBOW),
    patternColor(4278387100),
    parameter(1.0f),
    speed(1.0f),
    increment(0),
    refreshTimer(50, true) // 20Hz
{
    strip.begin();
    strip.setBrightness(255);
    this->brightness = min(1.0f, max(0.0f, brightness));
    this->masterBrightness = min(1.0f, max(0.0f, masterBrightness));
    clear(); // TODO clear leds after numPixels ?

    
    for (int i=0; i<numPixels; i++) noise[i] = random(255);

    refreshTimer.setCallback(std::bind(&LedStrip::refresh, this));
    refreshTimer.start();
}

void LedStrip::update()
{
    refreshTimer.update();
    // randomTimer.update();
}

void LedStrip::clear()
{
    memset(pixels, 0, numPixels * sizeof(CRGB));
    strip.clear();
    strip.show();
}


void LedStrip::show()
{
    for (int i = 0; i < numPixels;i++) strip.setPixelColor(i, 
        masterBrightness*brightness*Adafruit_NeoPixel::gamma8(pixels[i].r), 
        masterBrightness*brightness*Adafruit_NeoPixel::gamma8(pixels[i].g), 
        masterBrightness*brightness*Adafruit_NeoPixel::gamma8(pixels[i].b)
        );
    strip.show();
}

void LedStrip::fill(CRGB color, float multiplier)
{
    fill(CRGB(multiplier*color.red, multiplier*color.green, multiplier*color.blue));
}

void LedStrip::fill(CRGB color)
{
    for (int i = 0; i < numPixels;i++) pixels[i] = color;
}

void LedStrip::setPixel(uint8_t index, CRGB color)
{
    pixels[index] = color;
}

void LedStrip::setPixelWithHueShift(uint8_t index, CRGB color)
{
    CHSV hsv = rgb2hsv_approximate(color);
    hsv.hue += 128;  
    CRGB opposite = hsv;
    setPixel(index, opposite);
}

void LedStrip::setBrightness(float value)
{
    // TODO checkrange
    brightness = min(1.0f, max(0.0f, value));
}

void LedStrip::setPattern(LedPattern pattern, CRGB patternColor, float parameter, float speed, float brightness)
{
    setBrightness(brightness);
    setPattern(pattern, patternColor, parameter, speed);
}
    
void LedStrip::setPattern(LedPattern pattern, CRGB patternColor, float parameter, float speed)
{
    this->pattern = pattern;
    this->patternColor = patternColor;
    this->parameter =  min(1.0f, max(0.0f, parameter));;
    this->speed = min(10.0f, max(0.0f, speed));

    increment = 0;
    switch (pattern)
    {
    case SOLID:
        fill(patternColor);
        show();
        refreshTimer.stop();
        break;
        
    case BLINK:
        if (speed > 0.0f) 
        {
            int period = (int)1000.0/speed;
            refreshTimer.set(period/2, true);
            refreshTimer.start();
        }
    break;
        
    case OSCILLATOR:
        refreshTimer.set(33, true); // 30Hz
        refreshTimer.start();
    break;
        
    case CHASE:
        refreshTimer.set(33, true); // 30Hz
        refreshTimer.start();
    break;
        
    case RAINBOW:
        refreshTimer.set(33, true); // 30Hz
        refreshTimer.start();
    break;
        
    case RANDOM:
        refreshTimer.set(1000*speed, true);
        refreshTimer.start();
    break;
        
    case GAUGE:
        clear();
        for (int i = 0; i<parameter*numPixels; i++) setPixel(i, patternColor);
        show();
        refreshTimer.stop();
    break;
    
    default:
        break;
    }
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
        fill(increment%2==0?patternColor:0);
        show();
        break;

    case OSCILLATOR:
        fill(patternColor, 0.5*(1+cos(2.0f*3.14f*speed*millis()/1000.0)));
        show();
        break;
        
    case CHASE:
        value = (int)(increment *speed/10.0);
        for (int c=0; c<numPixels; c += parameter*20) setPixel((c+value)%numPixels, patternColor); 
        // fadeToBlackBy(pixels, numPixels, parameter*100);
        fadeToBlackBy(pixels, numPixels, 100);
        show();
    break;
    
    case RAINBOW:
        value = -(int)(increment*256*speed*5); // firstPixelHue = (increment*256)%(5*65536);
        strip.rainbow(value%(5*65536), 1, 255*parameter, masterBrightness*brightness*255, true);
        strip.show();
        break;
    
    case RANDOM:
        value = random(255); // seed
        for (int i=0; i<numPixels; i++) noise[(i+value)%255]<(parameter*255)?setPixel(i, patternColor):setPixel(i, 0);
        // for (int i; i<numPixels; i++) noise[(i+value)%255]<(parameter*255)?setPixel(i, patternColor):setPixelWithHueShift(i, patternColor);
        show();
        break;
        
    case GAUGE:
        break;
    }

}