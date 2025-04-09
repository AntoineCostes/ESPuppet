#include "Component.h"


Component::Component(const String &name, bool serialDebug, bool useInSequences) : name(name), serialDebug(serialDebug), useInSequences(useInSequences)
{
}

void Component::dbg(String message)
{
    if (serialDebug) Serial.println("[" + name + "] " + message);
}

void Component::log(String message)
{
    Serial.println("[" + name + "] " + message);
}

void Component::err(String message)
{
    Serial.println("[" + name + " ERROR] " + message);
}
