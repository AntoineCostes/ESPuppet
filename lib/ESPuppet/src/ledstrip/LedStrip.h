
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
    GAUGE
};

class LedStrip : public Component
{
    public:
        LedStrip(uint8_t pin, uint8_t numPixels, float brightness, neoPixelType type = NEO_GRB + NEO_KHZ800, float masterBrightness = 1.0f);   
        void update() override;
        void refresh();
        
        void clear();
        void show();
        void fill(CRGB color, float multiplier);
        void fill(CRGB color);
        // void fill(uint8_t r, uint8_t g, uint8_t b);
        void setPixel(uint8_t index, CRGB color);
        void setPixelWithHueShift(uint8_t index, CRGB color);
        // void setPixel(uint8_t index, uint8_t r, uint8_t g, uint8_t b);

        void setBrightness(float value);

        void setPattern(LedPattern pattern, CRGB patternColor, float parameter);
        void setPattern(LedPattern pattern, CRGB patternColor, float parameter, float speed);
        void setPattern(LedPattern pattern, CRGB patternColor, float parameter, float speed, float brightness);

    protected:
        Adafruit_NeoPixel strip;
        CRGB pixels[255];
        uint8_t noise[255];
        uint8_t numPixels;

        float masterBrightness;
        float brightness;
        float parameter;
        float speed;
        long increment;

        CRGB patternColor;
        LedPattern pattern;
        
        Timer refreshTimer;
        // Timer randomTimer;
};