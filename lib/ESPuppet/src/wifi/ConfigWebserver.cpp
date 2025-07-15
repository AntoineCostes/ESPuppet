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
  if (var == "DATE") return String(__DATE__);
  if (var == "CONFIG") return FileManager::getCurrentConfigName();
  if (var == "NICENAME") return FileManager::getCurrentConfigNiceName();
  if (var == "HOSTNAME") return FileManager::getCurrentConfigName()+".local";
  return "[???]";
}

String wifiProcessor(const String &var)
{
  if (var == "CURRENT_SSID") return FileManager::currentSSID();
  if (var == "SSID_LIST")
  {
    String options;
    for (int i = 0; i < NUM_CREDENTIALS; i++) 
      if (FileManager::getSSID(i) != "")
        options +=  "<option value='"+FileManager::getSSID(i)+"'>"+FileManager::getSSID(i)+"</option>\n" ;
    return options;

  }
  return "[???]";
}

String configProcessor(const String &var)
{
  if (var == "CONFIG") return FileManager::getCurrentConfigName();
  if (var == "CONFIG_OPTIONS")
  {
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
  else if (var.equals("chipid"))
    return String((uint32_t)ESP.getEfuseMac(), HEX);
  else if (var.equals("chiprev"))
    return (String)ESP.getChipRevision();
  else if (var.equals("idesize"))
    return (String)ESP.getFlashChipSize();
  else if (var.equals("flashsize"))
    return (String)ESP.getPsramSize();
  else if (var.equals("cpufreq"))
    return (String)ESP.getCpuFreqMHz();
  else if (var.equals("freeheap"))
    return (String)ESP.getFreeHeap();
  else if (var.equals("memsketch"))
    return (String)(ESP.getSketchSize()) + " / " + (String)(ESP.getFreeSketchSpace());
  else if (var.equals("memsketch_used"))
    return (String)(ESP.getSketchSize());
  else if (var.equals("memsketch_free"))
    return (String)(ESP.getFreeSketchSpace());
  else if (var.equals("memsmeter_max"))
    return (String)(ESP.getSketchSize() + ESP.getFreeSketchSpace());
  else if (var.equals("temp"))
    return (String)temperatureRead();
  else if (var.equals("stassid"))
    return FileManager::currentSSID();
  else if (var.equals("staip"))
    return WiFi.localIP().toString();
  else if (var.equals("stagw"))
    return WiFi.gatewayIP().toString();
  else if (var.equals("stasub"))
    return WiFi.subnetMask().toString();
  else if (var.equals("dnss"))
    return WiFi.dnsIP().toString();
  else if (var.equals("host"))
    return WiFi.getHostname();
  else if (var.equals("stamac"))
    return WiFi.macAddress();
  else if (var.equals("apip"))
    return WiFi.softAPIP().toString();
  else if (var.equals("apmac"))
    return WiFi.softAPmacAddress();
  else if (var.equals("aphost"))
    return WiFi.softAPgetHostname();
  else if (var.equals("apbssid"))
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

  // pages
  server->on("/", HTTP_GET, std::bind(&ConfigWebserver::serveIndex, this, std::placeholders::_1));
  server->on("/portal.css", HTTP_GET, std::bind(&ConfigWebserver::serveCSS, this, std::placeholders::_1));
  server->on("/wifi", HTTP_GET, std::bind(&ConfigWebserver::serveWifi, this, std::placeholders::_1));
  server->on("/info", HTTP_GET, std::bind(&ConfigWebserver::serveInfo, this, std::placeholders::_1));
  server->on("/reboot", HTTP_GET, std::bind(&ConfigWebserver::reboot, this, std::placeholders::_1));
  server->on("/config", HTTP_GET, std::bind(&ConfigWebserver::serveConfig, this, std::placeholders::_1));

  // get data
  server->on("/configfile", HTTP_GET, std::bind(&ConfigWebserver::handleGetConfigFile, this, std::placeholders::_1));

  // actions
  server->on("/wifiset", HTTP_POST, std::bind(&ConfigWebserver::handleWifiSet, this, std::placeholders::_1));
  server->on("/wifidelete", HTTP_POST, std::bind(&ConfigWebserver::handleWifiDelete, this, std::placeholders::_1));
  server->on("/wifisave", HTTP_POST, std::bind(&ConfigWebserver::handleWifiSave, this, std::placeholders::_1));
  server->on("/load", HTTP_POST, std::bind(&ConfigWebserver::handleLoadConfig, this, std::placeholders::_1));
  server->on("/download", HTTP_POST, std::bind(&ConfigWebserver::handleFileDownload, this, std::placeholders::_1));
  server->on("/delete", HTTP_POST, std::bind(&ConfigWebserver::handleFileDelete, this, std::placeholders::_1));
  
  // fails to redirect at the end
  // server->onFileUpload(std::bind(&ConfigWebserver::handleFileUpload, this, 
  //   std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, 
  //       std::placeholders::_5, std::placeholders::_6));
  server->on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) { request->redirect("/");}, 
  std::bind(&ConfigWebserver::handleFileUpload, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6)
);

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
  dbg("serve index");
  request->send(LittleFS, "/index.html", String(), false, generalProcessor);
}

void ConfigWebserver::serveCSS(AsyncWebServerRequest *request)
{
  // dbg("serve CSS");
  request->send(LittleFS, "/portal.css", "text/css");
}

void ConfigWebserver::serveInfo(AsyncWebServerRequest *request)
{
  // dbg("serve info");
  request->send(LittleFS, "/info.html", String(), false, infoProcessor);
}

void ConfigWebserver::serveWifi(AsyncWebServerRequest *request)
{
  // dbg("serve wifi");
  request->send(LittleFS, "/wifi.html", String(), false, wifiProcessor);
}

void ConfigWebserver::serveConfig(AsyncWebServerRequest *request)
{
  // dbg("serve config");
  request->send(LittleFS, "/config.html", String(), false, configProcessor);
}

void ConfigWebserver::reboot(AsyncWebServerRequest *request)
{
  dbg("REBOOT");
  shouldReboot = true;
  request->send(LittleFS, "/reboot.html", String(), false, generalProcessor);
}


void ConfigWebserver::handleWifiSet(AsyncWebServerRequest *request)
{
  if (request->hasParam("selected", true) )
  {
    const String ssid = request->getParam("selected", true)->value();
    if (FileManager::setWifiCredentials(ssid)) request->redirect("http://" + getIP().toString());
    else request->send(404, "text/plain", "Error: unknown ssid !");
  }
  else request->send(404, "text/plain", "Error: missing parameter");
}

void ConfigWebserver::handleWifiDelete(AsyncWebServerRequest *request)
{
  if (request->hasParam("selected", true) )
  {
    const String ssid = request->getParam("selected", true)->value();
    if (FileManager::deleteWifiCredentials(ssid)) request->redirect("http://" + getIP().toString());
    else request->send(404, "text/plain", "Error: unknown ssid !");
  }
  else request->send(404, "text/plain", "Error: missing parameter");
}

void ConfigWebserver::handleWifiSave(AsyncWebServerRequest *request)
{
  if (request->hasParam("ssid", true) && request->hasParam("pwd", true))
  {
    const String ssid = request->getParam("ssid", true)->value();
    const String pwd = request->getParam("pwd", true)->value();

    FileManager::registerWifiCredentials(ssid, pwd);
    request->redirect("http://" + getIP().toString());
  }
  else request->send(404, "text/plain", "Error: missing parameters");
}

void ConfigWebserver::handleLoadConfig(AsyncWebServerRequest *request)
{
  if (request->hasParam("selected", true) )
  {
    const String newConfig = request->getParam("selected", true)->value();
    dbg(newConfig);
    FileManager::setNewConfig(newConfig);
    shouldReboot = true;
    request->send(LittleFS, "/reboot.html", String(), false, generalProcessor);

  }
  else request->send(404, "text/plain", "Error: missing parameter");
}

void ConfigWebserver::handleGetConfigFile(AsyncWebServerRequest *request)
{
  if (request->hasParam("name") )
  {
    const String name = request->getParam("name")->value();
    request->send(200, "application/json", FileManager::openConfigFile(name).readString());
  } 
  else request->send(404, "text/plain", "Error: missing parameter");
}

void ConfigWebserver::handleFileDownload(AsyncWebServerRequest *request)
{
  if (request->hasParam("selected", true) )
  {
    const String name = request->getParam("selected", true)->value();
    request->send(LittleFS, "/"+String(ARDUINO_BOARD)+"/"+name+".json", String(), true);
  }
  else request->send(404, "text/plain", "Error: missing parameter");
}

void ConfigWebserver::handleFileDelete(AsyncWebServerRequest *request)
{
  if (request->hasParam("selected", true) )
  {
    const String name = request->getParam("selected", true)->value();

    if (FileManager::deleteConfigFile(name))
    {
      dbg("File deleted !");
      request->redirect("/");
    }
    else request->send(404, "text/plain", "Error: file was not found");
  }
  else request->send(404, "text/plain", "Error: missing parameter");
}

void ConfigWebserver::handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  // dbg("Client:" + request->client()->remoteIP().toString() + " " + request->url());

  if (!index) {
    dbg("Upload Start: " + String(filename));
    // open the file on first call and store the file handle in the request object
    request->_tempFile = FileManager::openFile("/"+String(ARDUINO_BOARD)+"/" + filename, true);
  }
  if (len) {
    // stream the incoming chunk to the opened file
    request->_tempFile.write(data, len);
    dbg("Writing file: " + String(filename) + String(index) + "/" + String(len));
  }
  if (final) {
    dbg("Upload Complete: " + String(filename) + "(" + String(index + len)+" bytes)");
    request->_tempFile.close();
  }
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

