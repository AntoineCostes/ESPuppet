#pragma once
#include "common/Includes.h"
#include "common/Module.h"
#include "LedStrip.h"

class LedModule : public Module
{
public:
    LedModule();

    void init();
    void update();

    void loadConfig(JsonObject const &config) override;
    void clear(uint8_t index);
    void clearAll();

    void setSolid(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
    void setWave(uint8_t index, uint8_t r, uint8_t g, uint8_t b, float frequency = 1.0f);
    void setBlink(uint8_t index, uint8_t r, uint8_t g, uint8_t b, float frequency = 1.0f);

protected:
    std::vector<LedStrip *> strips;
    void registerLedStrip(JsonObject const &config);
    void registerLedStrip(int pin, int numPixels, float brightness = 0.5f, neoPixelType type = NEO_GRB + NEO_KHZ800);

};