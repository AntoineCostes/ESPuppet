#include "ConfigWebserver.h"

String indexProcessor(const String &var)
{

  if (var == "TITLE") return "ESPuppet Config";
  if (var == "CONFIG")
  {
    Preferences prefs;
    prefs.begin("ESPuppet");
    String configFileName = prefs.getString("config", "notfound");
    prefs.end();
    return configFileName;
  }
  return "[???]";
}

String infoProcessor(const String &var)
{
  if (var.equals("uptime"))
    return (String)(millis() / 1000 / 60) + " mn " + (String)((millis() / 1000) % 60) + "s";
  if (var.equals("chipid"))
    return String((uint32_t)ESP.getEfuseMac(), HEX);
  if (var.equals("chiprev"))
    return (String)ESP.getChipRevision();
  if (var.equals("idesize"))
    return (String)ESP.getFlashChipSize();
  if (var.equals("flashsize"))
    return (String)ESP.getPsramSize();
  if (var.equals("cpufreq"))
    return (String)ESP.getCpuFreqMHz();
  if (var.equals("freeheap"))
    return (String)ESP.getFreeHeap();
  if (var.equals("memsketch"))
    return (String)(ESP.getSketchSize()) + " / " + (String)(ESP.getFreeSketchSpace());
  if (var.equals("memsketch_used"))
    return (String)(ESP.getSketchSize());
  if (var.equals("memsketch_free"))
    return (String)(ESP.getFreeSketchSpace());
  if (var.equals("memsmeter_max"))
    return (String)(ESP.getSketchSize() + ESP.getFreeSketchSpace());
  if (var.equals("temp"))
    return (String)temperatureRead();
  if (var.equals("conx"))
    return WiFi.isConnected() ? "Yes" : "No";
  if (var.equals("staip"))
    return WiFi.localIP().toString();
  if (var.equals("stagw"))
    return WiFi.gatewayIP().toString();
  if (var.equals("stasub"))
    return WiFi.subnetMask().toString();
  if (var.equals("dnss"))
    return WiFi.dnsIP().toString();
  if (var.equals("host"))
    return WiFi.getHostname();
  if (var.equals("stamac"))
    return WiFi.macAddress();
  if (var.equals("apip"))
    return WiFi.softAPIP().toString();
  if (var.equals("apmac"))
    return (String)WiFi.softAPmacAddress();
  if (var.equals("aphost"))
    return WiFi.softAPgetHostname();
  if (var.equals("apbssid"))
    return (String)WiFi.BSSIDstr();
  return "[???]";
}

ConfigWebserver::ConfigWebserver(bool serialDebug) : Component("webserver", serialDebug)
{
}

void ConfigWebserver::start()
{
  dnsServer = new DNSServer();
  dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
  // dnsServer->setTTL(300); TODO what is this for ?
  dnsServer->start(53, "*", WiFi.softAPIP());
  dbg("started on :" + WiFi.softAPIP().toString());

  server = new AsyncWebServer(80);
  // TODO check Filters example
  // does not work ?
  server->addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER); // only when requested from AP

  server->on("/", HTTP_GET, std::bind(&ConfigWebserver::serveIndex, this, std::placeholders::_1));
  server->on("/info", HTTP_GET, std::bind(&ConfigWebserver::serveInfo, this, std::placeholders::_1));
  server->on("/wifi", HTTP_GET, std::bind(&ConfigWebserver::serveWifi, this, std::placeholders::_1));
  server->on("/wifisave", HTTP_POST, std::bind(&ConfigWebserver::handleWifiSave, this, std::placeholders::_1));
  server->on("/portal.css", HTTP_GET, std::bind(&ConfigWebserver::serveCSS, this, std::placeholders::_1));
  // server->onNotFound(std::bind(&ConfigWebserver::redirect, this, std::placeholders::_1));
  server->onNotFound([](AsyncWebServerRequest *request)
                     {
                      Serial.println("NOT FOUND, host: "+ request->host()+", url:"+request->url()); 
                      request->send(404, "text/plain", "Not found"); });

  server->on("/success.txt", [](AsyncWebServerRequest *request)
             { 
              Serial.println("SUCCESS");
              request->send(200); });

  server->on("/canonical.html", [](AsyncWebServerRequest *request)
             { 
              // firefox captive portal call home
              Serial.println("canonical");
              request->redirect("http://" + WiFi.softAPIP().toString()); });

  server->on("/favicon.ico", [](AsyncWebServerRequest *request)
             { request->send(404); }); // webpage icon

  // handlers from https://wokwi.com/projects/394747420094281729
  server->on("/connecttest.txt", [](AsyncWebServerRequest *request)
             { 
              // windows 11 captive portal workaround
              Serial.println("connecttest");
              request->redirect("http://logout.net"); });

  server->on("/wpad.dat", [](AsyncWebServerRequest *request)
             { 
              // Honestly don't understand what this is but a 404 stops win 10 keep calling this repeatedly and panicking the esp32 :)
              Serial.println("WPAD");
              request->redirect("http://" + WiFi.softAPIP().toString()); });

  server->on("/generate_204", [](AsyncWebServerRequest *request)
             { 
               // android captive portal redirect
              Serial.println("204");
              request->redirect("http://" + WiFi.softAPIP().toString()); });

  server->on("/redirect", [](AsyncWebServerRequest *request)
             { 
              // microsoft redirect
              Serial.println("redrect");
              request->redirect("http://" + WiFi.softAPIP().toString()); });

  server->on("/hotspot-detect.html", [](AsyncWebServerRequest *request)
             { 
              // apple call home
              Serial.println("hotspot");
              request->redirect("http://" + WiFi.softAPIP().toString()); });

  server->on("/ncsi.txt", [](AsyncWebServerRequest *request)
             { 
              // windows call home
              Serial.println("ncsi");
              request->redirect("http://" + WiFi.softAPIP().toString()); });

  server->begin();
}

void ConfigWebserver::redirect(AsyncWebServerRequest *request)
{

  // server->addHeader("Location", "/",true); //Redirect to our html web page
  // server->_send(302, "text/plane","");

  // server->sendHeader("Location","http://" + WiFi.softAPIP().toString(), true); // @HTTPHEAD send redirect
  // server->send ( 302, "text/plain", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
  // server->client().stop();
}

void ConfigWebserver::serveIndex(AsyncWebServerRequest *request)
{
  dbg("SERVE INDEX");
  request->send(LittleFS, "/index.html", String(), false, indexProcessor);
}

void ConfigWebserver::serveInfo(AsyncWebServerRequest *request)
{
  dbg("serve info");
  request->send(LittleFS, "/info.html", String(), false, infoProcessor);
}

void ConfigWebserver::serveWifi(AsyncWebServerRequest *request)
{
  dbg("serve wifi");
  request->send(LittleFS, "/wifi.html", "text/html");
}

void ConfigWebserver::serveWifiSaved(AsyncWebServerRequest *request)
{
  dbg("serve wifi saved");
  request->send(LittleFS, "/wifisaved.html", "text/html");
}

void ConfigWebserver::handleWifiSave(AsyncWebServerRequest *request)
{
  dbg("SAVE WIFI");

  if (request->hasParam("ssid", true) && request->hasParam("pwd", true))
  {
    const String ssid = request->getParam("ssid", true)->value();
    const String pwd = request->getParam("pwd", true)->value();

    Preferences prefs;
    prefs.begin("wifi");
    prefs.putString("ssid", ssid.c_str());
    prefs.putString("pwd", pwd.c_str());
    prefs.end();

    dbg("new credentials: " + ssid + " / " + pwd);
  }

  request->redirect("http://" + WiFi.softAPIP().toString());
}

void ConfigWebserver::serveCSS(AsyncWebServerRequest *request)
{
  dbg("serve CSS");
  request->send(LittleFS, "/portal.css", "text/css");
}

void ConfigWebserver::stop()
{
  log("STOP");
  dnsServer->stop();
  server->end();
}

void ConfigWebserver::update()
{
  dnsServer->processNextRequest();
}