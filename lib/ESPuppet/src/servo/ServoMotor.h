#pragma once
#include "util/Includes.h"
#include "common/Component.h"

#define PWM_MIN 163
#define PWM_MAX 530

class ServoMotor : public Component
{
public:
    ServoMotor(String name, uint8_t pin, float min, float max, bool inverse, Adafruit_MS_PWMServoDriver *pwm);
    void update() override;

    // parameters
    float min;
    float max;
    bool inverse;

    void goTo(float relative);
    void goTo(float relative, uint32_t durationMs);

protected:
    Adafruit_MS_PWMServoDriver *pwm;
    Servo servo;

    int pin;

    float currentPosition;
    float startPosition;
    float targetPosition;
    uint32_t motionDurationMs;
    uint32_t motionStartMs;
    uint32_t lastMoveMs;

    float lerp(float a, float b, float f);
};
