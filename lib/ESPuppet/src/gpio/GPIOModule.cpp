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

    if (config["dout"]["pins"]) registerDigitalOutPins(config["dout"]["pins"].as<JsonArray>());

}

void GPIOModule::registerDigitalOutPins(JsonArray const &pins)
{
    for(JsonVariant pin : pins)
    {   
        if (Module::reservePin(pin))
        {
            digOutPins.emplace_back(pin);
            pinMode(pin, OUTPUT);
        } else
        err("cannot register digital out on pin #:"+String(pin));
    }
}

void GPIOModule::setDigitalOut(int index, bool value)
{
    if (index < 0 || index >= digOutPins.size())
    {
        err("invalid dout index: "+String(index)+ " while it should be between 0 and "+String(digOutPins.size()));
        return;
    }
    // dbg("set pin "+String(digOutPins[index])+value?"HIGH":"LOW");
    dbg("set dout #"+String(digOutPins[index]));
    if (value) dbg("HIGH");
    else dbg("LOW");
    digitalWrite(digOutPins[index], value);
}

void GPIOModule::handleOSCCommand(OSCMessage *command)
{
    if (command->match("/gpio/dout"))
    {
        if (command->size() == 2)
        {
            if (command->isInt(0) && command->isInt(1))
            {
                setDigitalOut(command->getInt(0), command->getInt(1)>0);
            }
            else if (command->isInt(0) && command->isBoolean(1))
            {
                setDigitalOut(command->getInt(0), command->getBoolean(1));
            }
        }
    } 
}