#pragma once
#include "util/Includes.h"
#include "common/Module.h"
#include "ServoMotor.h"

class ServoModule : public Module
{
public:
    ServoModule();
    void init() override;
    void update() override;
    void initMotorShield();
    
    void registerServo(uint8_t pin, float min, float max, bool inverse, bool multi);

    void loadConfig(JsonObject const &config) override;
    void handleOSCCommand(OSCMessage* command) override;

    void goTo(uint8_t index, float value);
    void goTo(uint8_t index, float value, float durationSec);

protected:
    std::vector<ServoMotor *> servos;
    Adafruit_MS_PWMServoDriver* pwm;
    void registerServo(JsonObject const &config);
};