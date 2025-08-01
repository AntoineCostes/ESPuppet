#pragma once
#include "util/Includes.h"
#include "common/Module.h"
#include "ODriveMotor.h"

#define BAUDRATE 115200

class ODriveModule : public Module
{
public:
    ODriveModule();

    void init() override;
    void update() override;
    
    void loadConfig(JsonObject const &config) override;
    void registerOdrive(int uartIndex, int rx, int tx);
    void handleOSCCommand(OSCMessage* command) override;

    void setPosition(int index, float value);
    void setVelocity(int index, float value);
    void setTorque(int index, float value);

protected:
    std::vector<ODriveMotor*> motors;
};