#pragma once
#include "util/Includes.h"
#include "common/Module.h"
#include "LedStrip.h"

class LedModule : public Module
{
public:
    LedModule();

    void init() override;
    void update();

    void loadConfig(JsonObject const &config) override;
    void clear(uint8_t index);
    void clearAll();

    void setSolidAll(uint8_t r, uint8_t g, uint8_t b);
    void setSolid(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
    void setWaveAll(uint8_t r, uint8_t g, uint8_t b, float frequency = 1.0f);
    void setWave(uint8_t index, uint8_t r, uint8_t g, uint8_t b, float frequency = 1.0f);
    void setBlinkAll(uint8_t r, uint8_t g, uint8_t b, float frequency = 1.0f);
    void setBlink(uint8_t index, uint8_t r, uint8_t g, uint8_t b, float frequency = 1.0f);

    void handleOSCCommand(OSCMessage* command) override;

protected:
    std::vector<LedStrip *> strips;
    void registerLedStrip(JsonObject const &config);
    void registerLedStrip(int pin, int numPixels, float brightness = 0.5f, neoPixelType type = NEO_GRB + NEO_KHZ800);

};