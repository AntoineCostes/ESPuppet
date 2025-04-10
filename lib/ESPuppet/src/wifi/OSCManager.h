#pragma once
#include "util/Includes.h"
#include "common/Component.h"
#include "common/EventBroadcaster.h"


class OSCManager : Component,
                   public EventBroadcaster<Command>
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
    // TODO make variadic function sendOSC(String address, OSCArgument args...)
    void sendMessage(OSCMessage &msg);

    String boardName;
    long oscPingTimeoutMs;
    bool oscSendDebug;
    bool oscReceiveDebug;

    long lastSentPingMs;
};