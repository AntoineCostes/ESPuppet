#pragma once
#include "common/Includes.h"
#include "common/Module.h"
#include "OSCManager.h"

class WifiModule : public Module
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
    long connectionTimeoutMs;
    
    long lastDisconnectTime;
    long lastConnectTime;
};