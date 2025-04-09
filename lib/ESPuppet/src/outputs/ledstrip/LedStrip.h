
#pragma once
#include "util/Includes.h"
#include "common/Component.h"


enum LedPattern
{
    STEADY,
    OSCILLATE,
    BLINK
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


        void setSolid(uint8_t r, uint8_t g, uint8_t b);
        void setWave(uint8_t r, uint8_t g, uint8_t b, float frequency = 1.0f);
        void notifyWave(uint8_t r, uint8_t g, uint8_t b, float frequency = 1.0f);
        void setBlink(uint8_t r, uint8_t g, uint8_t b, float frequency = 1.0f);
        void notifyBlink(uint8_t r, uint8_t g, uint8_t b, float frequency = 1.0f);

    protected:
        Adafruit_NeoPixel strip;
        uint8_t numPixels;
        float brightness;
        uint32_t patternColor;
        LedPattern pattern;
        float frequency;
        long lastLedChangeMs;
};
