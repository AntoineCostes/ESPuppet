#include "GPIOModule.h"

GPIOModule::GPIOModule() : Module("gpio")
{
}

void GPIOModule::init()
{
}

void GPIOModule::update()
{
    for (auto const &output : outputs)
        output->update();
}

void GPIOModule::loadConfig(JsonObject const &config)
{
    if (config) Serial.println("");
    serialDebug = config["serialDebug"] | false;

    // ceci pourrait être automatisé:
    // register des components du type indiqué, avec leur nom et leurs parameters
    // chaque parameter check son propre range
    if (config["outputs"].is<JsonObject>())
        for (JsonPair kv : config["outputs"].as<JsonObject>())
        {
            if (kv.value().is<JsonObject>())
                registerOutput(String(kv.key().c_str()), config["outputs"][kv.key()]);
        }
}

void GPIOModule::registerOutput(String name, JsonObject const &config)
{
    int pin = config["pin"] | -1;
    bool inverse = config["inverse"] | false;
    byte start = config["start"] | 0;

    if (GPIO_IS_VALID_GPIO(pin))
        registerOutput(name, pin, inverse, start);
    else
        err("cannot register output invalid pin");
}

void GPIOModule::registerOutput(String name, int pin, bool inverse, byte start)
{
    if (Module::reservePin(pin))
    {
        dbg("Register output pin #"+ String(pin));
        outputs.emplace_back(new Output(name, pin, start, inverse));
    }
    else
        err("cannot register output on pin #:" + pin);
}

void GPIOModule::toggleOutput(int index)
{
    if (index < 0 || index >= outputs.size())
    {
        err("invalid output index: " + String(index) + " while it should be between 0 and " + String(outputs.size()));
        return;
    }
    outputs[index]->toggle();
}

void GPIOModule::setOutput(int index, bool value)
{
    if (index < 0 || index >= outputs.size())
    {
        err("invalid output index: " + String(index) + " while it should be between 0 and " + String(outputs.size()));
        return;
    }
    outputs[index]->set(value);
}

void GPIOModule::setOutputPWM(int index, byte value)
{
    if (index < 0 || index >= outputs.size())
    {
        err("invalid output index: " + String(index) + " while it should be between 0 and " + String(outputs.size()));
        return;
    }
    outputs[index]->setPWM(value);
}

void GPIOModule::setOutputPeriod(int index, int value)
{
    if (index < 0 || index >= outputs.size())
    {
        err("invalid output index: " + String(index) + " while it should be between 0 and " + String(outputs.size()));
        return;
    }
    outputs[index]->setTogglePeriodMs((uint16_t)value);
} 

void GPIOModule::handleOSCCommand(OSCMessage *command)
{
    if (command->match("/gpio/output"))
    {
        if (command->size() == 1 && command->isInt(0))
        {
            toggleOutput(command->getInt(0));
        }
        if (command->size() == 2)
        {
            if (command->isInt(0) && command->isBoolean(1))
            {
                setOutput(command->getInt(0), command->getBoolean(1)); // boolean = on/off
            }
            else if (command->isInt(0) && command->isFloat(1))
            {
                setOutputPWM(command->getInt(0), (byte)(255*command->getFloat(1))); // float [0:1] = PWM
            }
            else if (command->isInt(0) && command->isInt(1))
            {
                setOutputPeriod(command->getInt(0), command->getInt(1)); // int = toggle period in ms
            }
        }
    }
}