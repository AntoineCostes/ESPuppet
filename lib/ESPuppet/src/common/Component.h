#pragma once
#include "util/Includes.h"

class Component
{
public:
    Component(const String &name, bool serialDebug = true, bool useInSequences = false);
    String name;
    bool useInSequences;

    virtual void update() = 0; // TODO make not pure virtual ?

protected:
    bool serialDebug;
    void dbg(String message);
    void log(String message);
    void err(String message);
};
