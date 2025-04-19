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

    void setPattern(LedPattern pattern, uint8_t r, uint8_t g, uint8_t b, float parameter = 1.0f);
    void setPattern(uint8_t index,LedPattern pattern, uint8_t r, uint8_t g, uint8_t b, float parameter = 1.0f);

    void handleOSCCommand(OSCMessage* command) override;

protected:
    std::vector<LedStrip *> strips;
    void registerLedStrip(JsonObject const &config);
    void registerLedStrip(int pin, int numPixels, float brightness = 0.5f, neoPixelType type = NEO_GRB + NEO_KHZ800);

};