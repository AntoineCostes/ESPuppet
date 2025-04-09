#pragma once
#include "common/Includes.h"
#include "common/Component.h"

class OSCManager : Component
{
public:
    OSCManager(uint16_t listeningPort,
               uint16_t targetPort,
               IPAddress targetIP,
               String boardName,
               long oscPingTimeoutMs,
               bool oscSendDebug,
               bool oscReceiveDebug);
    void update() override;

    void open();
    void close();
    
    uint16_t listeningPort;
    uint16_t targetPort;
    IPAddress targetIP;

protected:
    WiFiUDP udp;

    void sendOSC(String address);
    void sendMessage(OSCMessage &msg);
    
    String boardName;
    long oscPingTimeoutMs;
    bool oscSendDebug;
    bool oscReceiveDebug;

    long lastSentPingMs;
};