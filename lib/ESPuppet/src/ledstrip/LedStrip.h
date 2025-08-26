
#pragma once
#include "util/Includes.h"
#include "util/Timer.h"
#include "common/Component.h"


enum LedPattern
{
    SOLID,
    BLINK,
    OSCILLATOR,
    CHASE,
    RAINBOW,
    RANDOM,
    BIRANDOM,
    GAUGE
};

class LedStrip : public Component
{
    public:
        LedStrip(uint8_t pin, uint8_t numPixels, float brightness, neoPixelType type = NEO_GRB + NEO_KHZ800, float masterBrightness = 1.0f);   
        void update() override;
        void refresh();
        
        void clear();
        void fill(uint32_t color);
        void fill(uint32_t color, float multiplier);
        void fill(uint8_t r, uint8_t g, uint8_t b);
        void setPixel(uint8_t index, uint32_t color);
        void setPixel(uint8_t index, uint8_t r, uint8_t g, uint8_t b);

        void setBrightness(float value);

        void setPattern(LedPattern pattern, uint32_t patternColor, float parameter);
        void setPattern(LedPattern pattern, uint32_t patternColor, float parameter, float brightness);

    protected:
        Adafruit_NeoPixel strip;
        uint8_t numPixels;
        float brightness;
        float masterBrightness;
        uint32_t patternColor;
        LedPattern pattern;
        float parameter;
        long increment;
        Timer refreshTimer;
        Timer randomTimer;
};