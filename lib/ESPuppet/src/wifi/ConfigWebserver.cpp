#include "ConfigWebserver.h"

IPAddress ConfigWebserver::getIP()
{
  switch (WiFi.status())
  {
  case WL_NO_SHIELD: 
  return WiFi.softAPIP();

  case WL_CONNECTED:
  return WiFi.localIP();

  default:
    return IPAddress();
  }
}

String generalProcessor(const String &var)
{
  if (var == "BOARD") return String(ARDUINO_BOARD);
  if (var == "CONFIG") return FileManager::getCurrentConfigName();
  if (var == "HOSTNAME") return FileManager::getCurrentConfigName()+".local";
  return "[???]";
}

String configProcessor(const String &var)
{
  if (var == "CONFIG") return FileManager::getCurrentConfigName();
  if (var == "CONFIG_OPTIONS")
  {
    Serial.println("OPTIONS");
    String options;
    std::vector<String> configs = FileManager::getConfigNames();
    for (const String& name : configs) options +=  "<option value='"+name+"'>"+name+"</option>\n" ;
    return options;
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
  if (var.equals("stassid"))
  {
    
  Preferences prefs;
  prefs.begin("wifi");
  String ssid = prefs.getString("ssid", "");
  prefs.end();
  return ssid;
  }
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
    return WiFi.softAPmacAddress();
  if (var.equals("aphost"))
    return WiFi.softAPgetHostname();
  if (var.equals("apbssid"))
    return WiFi.BSSIDstr();
  return "[???]";
}

ConfigWebserver::ConfigWebserver(bool serialDebug) : Component("webserver", serialDebug)
{
}

void ConfigWebserver::start()
{
  dnsServer = new DNSServer();
  dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer->setTTL(6000); // default is 60, not sure what value is best
  dnsServer->start(53, "*", getIP());
  dbg("started on :" + getIP().toString());

  server = new AsyncWebServer(80);
  server->onNotFound([](AsyncWebServerRequest *request)
                     {
                      Serial.println("NOT FOUND, host: "+ request->host()+", url:"+request->url()); 
                      request->send(404, "text/plain", "Not found"); });

  server->on("/", HTTP_GET, std::bind(&ConfigWebserver::serveIndex, this, std::placeholders::_1));
  server->on("/portal.css", HTTP_GET, std::bind(&ConfigWebserver::serveCSS, this, std::placeholders::_1));
  server->on("/wifi", HTTP_GET, std::bind(&ConfigWebserver::serveWifi, this, std::placeholders::_1));
  server->on("/info", HTTP_GET, std::bind(&ConfigWebserver::serveInfo, this, std::placeholders::_1));
  server->on("/reboot", HTTP_GET, std::bind(&ConfigWebserver::reboot, this, std::placeholders::_1));
  server->on("/config", HTTP_GET, std::bind(&ConfigWebserver::serveConfig, this, std::placeholders::_1));

  server->on("/wifisave", HTTP_POST, std::bind(&ConfigWebserver::handleWifiSave, this, std::placeholders::_1));
  server->on("/loadconfig", HTTP_POST, std::bind(&ConfigWebserver::handleLoadConfig, this, std::placeholders::_1));
  
  server->onFileUpload(std::bind(&ConfigWebserver::handleConfigUpload, this, 
    std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, 
        std::placeholders::_5, std::placeholders::_6));
        

  //redirections for captive portal
  server->on("/success.txt", [](AsyncWebServerRequest *request)
             { 
              Serial.println("SUCCESS");
              request->send(200); });

  server->on("/canonical.html", [](AsyncWebServerRequest *request)
             { 
              // firefox captive portal call home
              Serial.println("canonical");
              request->redirect("http://" + WiFi.softAPIP().toString()); });

  server->on("/monitor.html", [](AsyncWebServerRequest *request)
             { 
              Serial.println("monitor");
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

  server->on("/redirect/internal", [](AsyncWebServerRequest *request)
             { 
              // microsoft redirect
              Serial.println("redrect internal");
              request->redirect("http://" + WiFi.softAPIP().toString()); });

  server->on("/redirect/external", [](AsyncWebServerRequest *request)
             { 
              // microsoft redirect
              Serial.println("redrect internal");
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

void ConfigWebserver::serveIndex(AsyncWebServerRequest *request)
{
  dbg("SERVE INDEX");
  request->send(LittleFS, "/index.html", String(), false, generalProcessor);
}

void ConfigWebserver::serveInfo(AsyncWebServerRequest *request)
{
  dbg("serve info");
  request->send(LittleFS, "/info.html", String(), false, infoProcessor);
}

void ConfigWebserver::serveWifi(AsyncWebServerRequest *request)
{
  dbg("serve wifi");
  request->send(LittleFS, "/wifi.html", String(), false, generalProcessor);
}

void ConfigWebserver::serveConfig(AsyncWebServerRequest *request)
{
  dbg("serve config");
  request->send(LittleFS, "/config.html", String(), false, configProcessor);
}

void ConfigWebserver::reboot(AsyncWebServerRequest *request)
{
  dbg("REBOOT");
  shouldReboot = true;
  request->send(LittleFS, "/reboot.html", String(), false, generalProcessor);
}

void ConfigWebserver::handleWifiSave(AsyncWebServerRequest *request)
{
  if (request->hasParam("ssid", true) && request->hasParam("pwd", true))
  {
    const String ssid = request->getParam("ssid", true)->value();
    const String pwd = request->getParam("pwd", true)->value();

    Preferences prefs;
    prefs.begin("wifi");
    prefs.putString("ssid", ssid.c_str());
    prefs.putString("pwd", pwd.c_str());
    prefs.end();

    dbg("new wifi credentials: " + ssid + " / " + pwd);
    request->redirect("http://" + getIP().toString());
  }
  else request->send(404, "text/plain", "Not found");
}

void ConfigWebserver::handleLoadConfig(AsyncWebServerRequest *request)
{
  dbg("load config");
  
  if (request->hasParam("config", true) )
  {
    const String newConfig = request->getParam("config", true)->value();
    dbg(newConfig);
    FileManager::setNewConfig(newConfig);
    shouldReboot = true;
    request->send(LittleFS, "/reboot.html", String(), false, generalProcessor);

  }
  else request->send(404, "text/plain", "Not found");
}


void ConfigWebserver::handleConfigUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  dbg("UPLOAD");
  String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
  Serial.println(logmessage);

  if (!index) {
    logmessage = "Upload Start: " + String(filename);
    // open the file on first call and store the file handle in the request object
    request->_tempFile = FileManager::openFile("/" + filename, true);
    Serial.println(logmessage);
  }

  if (len) {
    // stream the incoming chunk to the opened file
    request->_tempFile.write(data, len);
    logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
    Serial.println(logmessage);
  }

  if (final) {
    logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
    // close the file handle as the upload is now done
    request->_tempFile.close();
    Serial.println(logmessage);
    request->redirect("/");
  }
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

  if (shouldReboot) 
  {
  long now = millis();
    while (millis() < now + 1000) 
    {
      dnsServer->processNextRequest();
      delay(10);
    }
    ESP.restart();
  }
}

