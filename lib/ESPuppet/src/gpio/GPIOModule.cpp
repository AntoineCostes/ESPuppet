#include "GPIOModule.h"

GPIOModule::GPIOModule() : Module("gpio")
{
}

void GPIOModule::init()
{
}

void GPIOModule::update()
{
}

void GPIOModule::loadConfig(JsonObject const &config)
{
    serialDebug = config["serialDebug"] | false;

    if (config["dout"]["pins"])
        registerDigitalOutPins(config["dout"]["pins"].as<JsonArray>());
}

// TODO start & inverse parameters
void GPIOModule::registerDigitalOutPins(JsonArray const &pins)
{
    for (JsonVariant pin : pins)
    {
        if (Module::reservePin(pin))
        {
            digOutPins.emplace_back(pin);
            pinMode(pin, OUTPUT);
            digOutValues.emplace_back(HIGH);
            digitalWrite(pin, HIGH); // FIXME add inverse parameter
        }
        else
        {
            String p = pin.as<String>();
            err("cannot register digital out on pin #:" + p);
        }
    }
}

void GPIOModule::toggleDigitalOut(int index)
{
    if (index < 0 || index >= digOutPins.size())
    {
        err("invalid dout index: " + String(index) + " while it should be between 0 and " + String(digOutPins.size()));
        return;
    }
    digOutValues[index] = !digOutValues[index]; // FIXME add inverse parameter
    dbg("toggle dout #" + String(digOutPins[index]) + " to " + (digOutValues[index] ? "HIGH" : "LOW"));
    digitalWrite(digOutPins[index], digOutValues[index]);
}

void GPIOModule::setDigitalOut(int index, bool value)
{
    if (index < 0 || index >= digOutPins.size())
    {
        err("invalid dout index: " + String(index) + " while it should be between 0 and " + String(digOutPins.size()));
        return;
    }
    digOutValues[index] = !value; // FIXME add inverse parameter
    dbg("set dout #" + String(digOutPins[index]) + " to " + (digOutValues[index] ? "HIGH" : "LOW"));
    digitalWrite(digOutPins[index], digOutValues[index]);
}

void GPIOModule::handleOSCCommand(OSCMessage *command)
{
    if (command->match("/gpio/dout"))
    {

        if (command->size() == 1 && command->isInt(0))
        {
            toggleDigitalOut(command->getInt(0));
        }
        if (command->size() == 2)
        {
            if (command->isInt(0) && command->isInt(1))
            {
                setDigitalOut(command->getInt(0), command->getInt(1) > 0);
            }
            else if (command->isInt(0) && command->isBoolean(1))
            {
                setDigitalOut(command->getInt(0), command->getBoolean(1));
            }
        }
    }
}