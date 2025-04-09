#pragma once
#include "util/Includes.h"
#include "common/Module.h"
#include "OSCManager.h"

class WifiModule : public Module, public EventBroadcaster<Command>
{
public:
    WifiModule();

    void init() override;
    void update() override;
    void loadConfig(JsonObject const &config) override;
    String getDefaultBoardName();

    void initAP();
    void initSTA();
    void initMDNS();

    OSCManager *osc;

    enum STATUS
    {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
    } status = DISCONNECTED;

protected:
    String boardName;
    DNSServer dnsServer;
    void WiFiEvent(WiFiEvent_t event, arduino_event_info_t info);
    void gotOSCCommand(const Command &command);
    long connectionTimeoutMs;
    
    long lastDisconnectTime;
    long lastConnectTime;
};