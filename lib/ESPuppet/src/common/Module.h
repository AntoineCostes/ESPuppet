#pragma once
#include "util/Includes.h"
#include "Component.h"

// A module deals with a specific specific kind of hardware, 
// be it an actuator, a sensor, a shield or an internal component 
// (like an ADC or a Wifi chip).
// A module might configure a depdendency library in its init() method,
// and instantiate dedicated components according to a JSON document.
// This abstract class mainly deals with keeping track of the pin usage.
class Module : public Component
{
public:
    Module(const String &name, bool serialDebug = false);

    virtual void init() = 0; // TODO make not pure virtual ?
    virtual void loadConfig(JsonObject const &config) = 0;

    bool reservePin(int pin);
    static std::set<int> reservedPins;

    virtual void handleCommand(const Command& command);

protected:
    std::vector<std::unique_ptr<Component>> props;
    bool registerProp(Component* comp, std::set<int> reservedPins);
};