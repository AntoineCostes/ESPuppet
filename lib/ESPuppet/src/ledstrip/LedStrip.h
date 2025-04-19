
#pragma once
#include "util/Includes.h"
#include "common/Component.h"


enum LedPattern
{
    SOLID,
    BLINK,
    OSCILLATOR,
    CHASE,
    RAINBOW
};

class LedStrip : public Component
{
    public:
        LedStrip(uint8_t pin, uint8_t numPixels, float brightness, neoPixelType type = NEO_GRB + NEO_KHZ800);   
        void update() override;
        
        void clear();
        void fill(uint32_t color);
        void fill(uint32_t color, float multiplier);
        void fill(uint8_t r, uint8_t g, uint8_t b);

        void setBrightness(float value);

        void setPattern(LedPattern pattern, uint32_t patternColor, float parameter);

    protected:
        Adafruit_NeoPixel strip;
        uint8_t numPixels;
        float brightness;
        uint32_t patternColor;
        LedPattern pattern;
        float parameter;
        long lastLedChangeMs;
        long increment;
};
