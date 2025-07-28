#pragma once
#include "util/Includes.h"
#include "common/Module.h"

class GPIOModule : public Module
{
public:
GPIOModule();

    void init() override;
    void update();
    
    void registerDigitalOutPins(std::set<int> pin);

    void loadConfig(JsonObject const &config) override;
    void handleOSCCommand(OSCMessage* command) override;

    void setAnalogOut(int index, float value);
    void setDigitalOut(int index, bool value);
    void toggleDigitalOut(int index);

protected:
    std::vector<int> digOutPins;
    std::vector<bool> digOutValues;
    void registerDigitalOutPins(JsonArray const &pins);
};