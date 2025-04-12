#pragma once
#include "util/Includes.h"
#include "common/Component.h"

class ConfigWebserver : public Component
{
public:
    ConfigWebserver(bool serialDebug);

    void update() override;

    void start();
    void stop();

protected:
    DNSServer* dnsServer;
    AsyncWebServer* server;
  void serveIndex(AsyncWebServerRequest *request);
  void serveCustomIndex(AsyncWebServerRequest *request);
  void serveInfo(AsyncWebServerRequest *request);
  void serveWifi(AsyncWebServerRequest *request);


};


class CaptiveRequestHandler : public AsyncWebHandler {
    public:
      CaptiveRequestHandler() {}
      virtual ~CaptiveRequestHandler() {}
  
      bool canHandle(__unused AsyncWebServerRequest* request) {
        // request->addInterestingHeader("ANY");
        Serial.println("can Handle");
        return true;
      }
  
      void handleRequest(AsyncWebServerRequest* request) {
        Serial.println("handle");
        AsyncResponseStream* response = request->beginResponseStream("text/html");
        response->print("<!DOCTYPE html><html><head><title>Captive Portal</title></head><body>");
        response->print("<p>This is out captive portal front page.</p>");
        response->printf("<p>You were trying to reach: http://%s%s</p>", request->host().c_str(), request->url().c_str());
        response->printf("<p>Try opening <a href='http://%s'>this link</a> instead</p>", WiFi.softAPIP().toString().c_str());
        response->print("</body></html>");
        request->send(response);
      }
  };
  