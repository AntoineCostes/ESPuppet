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
    void sendMessage(OSCMessage &msg);
    void sendMessage(const String &source, const String &command, var *data, int numData);
    OSCMessage createMessage(const String &source, const String &command, const var *data, int numData, bool addID);
    var OSCArgumentToVar(OSCMessage &m, int index);

    String boardName;
    long oscPingTimeoutMs;
    bool oscSendDebug;
    bool oscReceiveDebug;

    long lastSentPingMs;
};