#include "Module.h"


Module::Module(const String &name, bool serialDebug) : Component(name, serialDebug)
{
}

std::set<int> Module::reservedPins = {};

void Module::handleOSCCommand(OSCMessage* command)
{
    log("OSC command handler not overloaded");
}

bool Module::reservePin(int pin)
{
    // for (int pin : pins)
    // {
        if (Module::reservedPins.find(pin) != Module::reservedPins.end())
        {
            err("pin #"+String(pin)+" is not available !");
            return false;
        }
    // }
    // for (int pin : pins)
    // {
        dbg("-------------- registered pin #"+String(pin));
        Module::reservedPins.insert(pin);
    // }
    return true;
}
