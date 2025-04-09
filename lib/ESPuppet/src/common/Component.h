#pragma once
#include "util/Includes.h"

class Component
{
public:
    Component(const String &name, bool serialDebug = false, bool useInSequences = false);
    String name;
    bool useInSequences;

    virtual void update() = 0; // TODO make not pure virtual ?

protected:
    bool serialDebug;
    void dbg(String message);
    void log(String message);
    void err(String message);
};

class Command
{
public:
    String target;
    String command;
    var *data;
    int numData;
    Command(String target, String command, var *data, int numData) : target(target), command(command), data(data), numData(numData){}
};