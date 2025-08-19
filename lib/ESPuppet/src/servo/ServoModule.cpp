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
    if (config) Serial.println("");
    serialDebug = config["serialDebug"] | false;

    for (JsonPair kv : config)
        if (kv.value().is<JsonObject>())
            registerServo(String(kv.key().c_str()), config[kv.key()]);
}

void ServoModule::registerServo(String name, JsonObject const &config)
{
    int pin = config["pin"] | -1;
    float min = config["min"] | static_cast<float>(0.0);
    float max = config["max"] | static_cast<float>(1.0);
    float start = config["start"] | static_cast<float>(0.5);
    bool inverse = config["inverse"] | false;
    bool multi = config["multi"] | false;

    if (pin >= 0 && min >= 0.0 && max >= 0.0)
    {
        registerServo(name, pin, min, max, inverse, multi);
        set(servos.size() - 1, start);
    }
    else
        err("cannot register servo, pin (" + String(pin) + "), min (" + String(min) + ") and max (" + String(max) + ") should be positive !");
}

void ServoModule::registerServo(String name, uint8_t pin, float min, float max, bool inverse, bool multi)
{
    if (multi)
    {
        if (pwm == nullptr) initMotorShield();
        servos.emplace_back(new ServoMotor(name, pin, min, max, inverse, pwm));
    }
    else
    {
        if (Module::reservePin(pin))
        {
            log("Register servo on pin #" + String(pin));
            servos.emplace_back(new ServoMotor(name, pin, min, max, inverse, nullptr));
        }
        else
            err("cannot register servo, pin" + String(pin) + " is reserved");
    }
}

void ServoModule::set(uint8_t index, float value)
{
    if (index < 0 || index >= servos.size())
    {
        err("invalid servo index: " + String(index) + " it should be between 0 and " + String(servos.size() - 1));
        return;
    }
    servos[index]->goTo(value);
}

void ServoModule::move(uint8_t index, float value, float durationSec)
{
    if (index < 0 || index >= servos.size())
    {
        err("invalid servo index: " + String(index) + " while it should be between 0 and " + String(servos.size() - 1));
        return;
    }
    servos[index]->goTo(value, durationSec * 1000);
}

void ServoModule::handleOSCCommand(OSCMessage *command)
{
    if (command->fullMatch("/servo/set"))
    {
        // index, position
        if (command->size() == 2 && command->isInt(0) && command->isFloat(1))
        {
            int index = command->getInt(0);
            float position = command->getFloat(1);
            set(index, position);
        }
        else // position, position, position...
            for (int i = 0; i < command->size(); i++)
                if (command->isFloat(i))
                    set(i, command->getFloat(i));

    }
    else if (command->fullMatch("/servo/move"))
    {
        // duration, index, position
        if (command->size() == 3 && command->isFloat(0) && command->isInt(1) && command->isFloat(2))
        {
            float duration = command->getFloat(0);
            int index = command->getInt(1);
            float position = command->getFloat(2);
            move(index, position, duration);
        }
        else if (command->size() >= 2 && command->isFloat(0))
        {
            // duration, position, position, position...
            float duration = command->getFloat(0);
            for (int i = 1; i < command->size(); i++)
                if (command->isFloat(i) )
                    move(i-1, command->getFloat(i), duration);
        }
    }
}