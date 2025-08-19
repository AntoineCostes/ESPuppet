#pragma once
#include "util/Includes.h"
#include "common/Module.h"
#include "OSCManager.h"
#include "ConfigWebserver.h"

class WifiModule : public Module, public EventBroadcaster<Command>
{
public:
    WifiModule();

    void init() override;
    void update() override;
    void loadConfig(JsonObject const &config) override;

    void initAP();
    void initSTA();
    void initZeroConf();
    void goOnAir();
    void goOffAir();

    OSCManager *osc;
    ConfigWebserver *configServer;

protected:
    void WiFiEvent(WiFiEvent_t event, arduino_event_info_t info);
    void gotOSCCommand(const Command &command);
    bool hasWebServer;
    bool isAP();

    long configPortalTimeoutMs;
    long configPortalStartTimeMs;
    
    int connectionAttempts;
    int numDisconnections;
    long disconnectedTimeoutMs;
    long lastDisconnectionTimeMs;
    bool isConnecting;
    bool onAir;
};