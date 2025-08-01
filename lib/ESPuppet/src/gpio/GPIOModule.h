#pragma once
#include "util/Includes.h"
#include "common/Module.h"
#include "Output.h"

class GPIOModule : public Module
{
public:
    GPIOModule();

    void init() override;
    void update() override;
    
    void loadConfig(JsonObject const &config) override;
    void handleOSCCommand(OSCMessage* command) override;

    void setOutputPWM(int index, byte value);
    void setOutput(int index, bool value);
    void setOutputPeriod(int index, int value);
    void toggleOutput(int index);

protected:
    void registerOutput(String name, JsonObject const &config);
    void registerOutput(String name, int pin, bool inverse, byte start);
    std::vector<Output*> outputs;
};