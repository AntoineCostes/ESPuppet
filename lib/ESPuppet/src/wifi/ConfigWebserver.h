#pragma once
#include "util/Includes.h"
#include "common/Component.h"
#include "common/FileManager.h"

class ConfigWebserver : public Component
{
public:
    ConfigWebserver(bool serialDebug);

    void update() override;

    void start();
    void stop();
    static IPAddress getIP();

protected:
    DNSServer* dnsServer;
    AsyncWebServer* server;
    void serveIndex(AsyncWebServerRequest *request);
    void serveCSS(AsyncWebServerRequest *request);
    void serveConfig(AsyncWebServerRequest *request);
    void handleLoadConfig(AsyncWebServerRequest *request);
    void handleConfigUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
    void serveWifi(AsyncWebServerRequest *request);
    void handleWifiSave(AsyncWebServerRequest *request);
    void serveInfo(AsyncWebServerRequest *request);
    void reboot(AsyncWebServerRequest *request);
    
    bool shouldReboot;

};
