#pragma once
#include "util/Includes.h"
#include "common/Module.h"
#include "LedStrip.h"

class LedModule : public Module
{
public:
    LedModule();

    void init() override;
    void update() override;

    void registerLedStrip(int pin, int numPixels, float brightness = 0.5f, neoPixelType type = NEO_GRB + NEO_KHZ800);

    void loadConfig(JsonObject const &config) override;
    void handleOSCCommand(OSCMessage* command) override;
    void clear(uint8_t index);
    void clearAll();

    void setPattern(LedPattern pattern, uint8_t r, uint8_t g, uint8_t b, float parameter, float brightness);
    void setPattern(uint8_t index,LedPattern pattern, uint8_t r, uint8_t g, uint8_t b, float parameter, float brightness);

protected:
    std::vector<LedStrip *> strips;
    void registerLedStrip(JsonObject const &config);
    
};