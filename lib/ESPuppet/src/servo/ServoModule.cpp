#include "ServoModule.h"

ServoModule::ServoModule() : Module("servo")
{
}

void ServoModule::init()
{
}

void ServoModule::update()
{
    for (auto const &servo : servos)
        servo->update();
}

void ServoModule::initMotorShield()
{
    if (Module::reservePin(22) && Module::reservePin(23))
    {
        pwm = new Adafruit_MS_PWMServoDriver();
        pwm->begin();
        pwm->setPWMFreq(60); // Analog servos run at ~60 Hz updates
    }
    else
        err("motor shield needs pins 22 & 23 for SCL/SDA");
}

void ServoModule::loadConfig(JsonObject const &config)
{
    serialDebug = config["serialDebug"] | false;

    for (JsonPair kv : config)
    {
        if (kv.value().is<JsonObject>())
            registerServo(config[kv.key()]);
    }
}

void ServoModule::registerServo(JsonObject const &config)
{
    int pin = config["pin"] | -1;
    float min = config["min"];
    float max = config["max"]; // FIXME how to set default value ?
    float start = config["start"];
    bool inverse = config["inverse"] | false;
    bool multi = config["multi"] | false;

    if (pin >= 0 && min >= 0 && max >= 0)
    {
        registerServo(pin, min, max, inverse, multi);
        if (start)
            goTo(servos.size() - 1, start);
    }
    else
        err("cannot register servo, pin (" + String(pin) + "), min (" + String(min) + ") and max (" + String(max) + ") should be positive !");
}

void ServoModule::registerServo(uint8_t pin, float min, float max, bool inverse, bool multi)
{
    if (multi)
    {
        if (pwm == nullptr)
            initMotorShield();
        servos.emplace_back(new ServoMotor(pin, min, max, inverse, pwm));
    }
    else
    {
        if (Module::reservePin(pin))
        {
            dbg("Register servo on pin #" + String(pin));
            servos.emplace_back(new ServoMotor(pin, min, max, inverse, nullptr));
        }
        else
            err("cannot register ledstrip, pin" + String(pin) + " is reserved");
    }
}

void ServoModule::goTo(uint8_t index, float value)
{
    if (index < 0 || index >= servos.size())
    {
        err("invalid servo index: " + String(index) + " it should be between 0 and " + String(servos.size()));
        return;
    }
    servos[index]->goTo(value);
}

void ServoModule::goTo(uint8_t index, float value, float durationSec)
{
    if (index < 0 || index >= servos.size())
    {
        err("invalid servo index: " + String(index) + " while it should be between 0 and " + String(servos.size()));
        return;
    }
    servos[index]->goTo(value, durationSec * 1000);
}

void ServoModule::handleOSCCommand(OSCMessage *command)
{
    if (command->match("/servo/set"))
    {
        // index, position
        if (command->size() == 2 && command->isInt(0) && command->isFloat(1))
        {
            int index = command->getInt(0);
            float position = command->getFloat(1);
            goTo(index, position);
        }
        else // position, position, position...
            for (int i = 0; i < command->size(); i++)
                if (command->isFloat(i))
                    goTo(i, command->getFloat(i));

    }
    else if (command->match("/servo/move"))
    {
        // duration, index, position
        if (command->size() == 3 && command->isFloat(0) && command->isInt(1) && command->isFloat(2))
        {
            float duration = command->getFloat(0);
            int index = command->getInt(1);
            float position = command->getFloat(2);
            goTo(index, position, duration);
        }
        else if (command->size() >= 2 && command->isFloat(0))
        {
            // duration, position, position, position...
            float duration = command->getFloat(0);
            for (int i = 1; i < command->size(); i++)
                if (command->isFloat(i) )
                    goTo(i-1, command->getFloat(i), duration);
        }
    }
}