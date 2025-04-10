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
    OSCMessage* command;
    Command(OSCMessage* command) : command(command){
      String address = String(command->getAddress()).substring(1);
      int separatorIndex = address.indexOf('/');
      targetModule = separatorIndex == -1 ? "root" : address.substring(0, separatorIndex); 
      targetComponent = address.substring(separatorIndex + 1);
    }
    String targetModule;
    String targetComponent;
};