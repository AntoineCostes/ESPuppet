#include "ServoMotor.h"

ServoMotor::ServoMotor(uint8_t pin,
                       float min,
                       float max,
                       bool inverse,
                       Adafruit_MS_PWMServoDriver *pwm) : Component("servo_" + String(pin), true),
                                                          pin(pin),
                                                          min(min),
                                                          max(max),
                                                          inverse(inverse),
                                                          pwm(pwm),
                                                          startPosition(-1),
                                                          targetPosition(-1),
                                                          motionDurationMs(0),
                                                          motionStartMs(0)
{
    if (pwm)
    {
        // goTo(0.0f);
    }
    else
    {
        servo.attach(pin);
        currentPosition = servo.read();
    }

    if (this->min < 0.0f)
        this->min = 0.0f;
    if (this->max > 1.0f)
        this->max = 1.0f;
}

void ServoMotor::update()
{
    if (motionDurationMs > 0)
    {
        if (millis() > motionStartMs + motionDurationMs)
        {
            goTo(targetPosition); // in case we didn't reach yet
            motionDurationMs = 0; // stop moving
        }
        else if (millis() > lastMoveMs + 10)
        {
            goTo(lerp(startPosition, targetPosition, (float)(millis() - motionStartMs) / (float)motionDurationMs));
            lastMoveMs = millis();
        }
    }
}

float ServoMotor::lerp(float a, float b, float f)
{
    return a * (1.0 - f) + (b * f);
}

void ServoMotor::goTo(float relative, uint32_t durationMs)
{
    if (durationMs < 50)
        goTo(relative);
    else
    {
        startPosition = currentPosition;
        targetPosition = relative;
        motionDurationMs = durationMs;
        motionStartMs = millis();
        // lastMoveMs = millis();
    }
}

void ServoMotor::goTo(float relative)
{
    if (relative < 0.0f || relative > 1.0f)
    {
        err("servo position: " + String(relative) + " should be [0:1]");
        return;
    }
    currentPosition = relative;
    lastMoveMs = millis();

    float targetPosition = min + relative * (max - min);
    if (inverse)
        targetPosition = max + relative * (min - max);

    dbg("go to " + String(targetPosition) + " / " + String(PWM_MIN + (PWM_MAX - PWM_MIN) * targetPosition));

    if (pwm)
        pwm->setPWM(pin, 0, PWM_MIN + (PWM_MAX - PWM_MIN) * targetPosition);
    else
        servo.writeMicroseconds(DEFAULT_uS_LOW + targetPosition * (DEFAULT_uS_HIGH - DEFAULT_uS_LOW));
}
