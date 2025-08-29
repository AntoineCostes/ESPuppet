#pragma once
#include "util/Includes.h"
#include "util/EventBroadcaster.h"
#include "common/Component.h"
#include "common/FileManager.h"

class Command
{
public:
    OSCMessage* command;
    Command(OSCMessage* command) : command(command){
        // FIXME don't use address elsewhere
        // remove "/BOARD_NAME"
        String address = String(command->getAddress()).substring(1+FileManager::getCurrentConfigName().length()); 
        command->setAddress(address.c_str());
        
        int separatorIndex = address.indexOf('/', 1); // skip first /
        targetModule = separatorIndex == -1 ? "root" : address.substring(1, separatorIndex); 
        targetComponent = address.substring(separatorIndex + 1);

        // Serial.println(targetModule);
        // Serial.println(targetComponent);
        // Serial.println(action);
    }
    String targetModule;
    String targetComponent;
    // String action;
};

class OSCManager : Component,
                   public EventBroadcaster<Command>
{
public:
    OSCManager(uint16_t listeningPort,
               uint16_t targetPort,
               IPAddress targetIP,
               bool broadcast,
               long oscPingTimeoutMs,
               bool oscSendDebug,
               bool oscReceiveDebug);
    void update() override;

    void open();//IPAddress broadcastIP, IPAddress gatewayIP);
    void close();

    uint16_t listeningPort;
    uint16_t targetPort;
    IPAddress targetIP;
    
    bool doBroadcast;
    bool isOpen;

protected:
    WiFiUDP udp;
    // IPAddress broadcastIP;
    // IPAddress gatewayIP;

    void sendOSC(String address);
    // TODO make variadic function sendOSC(String address, OSCArgument args...)
    void sendMessage(OSCMessage &msg, bool broadcast, bool silent = false);
    void udpSendFailed();

    long oscPingTimeoutMs;
    bool oscSendDebug;
    bool oscReceiveDebug;

    long lastSentPingMs;
};