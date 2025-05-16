#include "WifiModule.h"

WifiModule::WifiModule() : Module("wifi")
{
}

void WifiModule::init()
{
  // TODO declare parameters
  serialDebug = true;
  connectionTimeoutMs = 5000;
  configPortalTimeoutMs = 2*60*1000;

  lastConnectTime = millis();
  lastDisconnectTime = millis();

  WiFi.onEvent(std::bind(&WifiModule::WiFiEvent, this, std::placeholders::_1, std::placeholders::_2));

  WiFi.setAutoReconnect(true);
  WiFi.setSleep(false);
  // WiFi.setTxPower(WIFI_POWER_19dBm); TODO parameter
}

void WifiModule::loadConfig(JsonObject const &config)
{
  serialDebug = config["serialDebug"] | serialDebug;
  connectionTimeoutMs = config["connectionTimeoutMs"] | connectionTimeoutMs;
  boardName = config["boardName"] | "default";
  boardName.replace(" ", "_");

  if (config["osc"])
  {
    uint16_t listeningPort = config["osc"]["listeningPort"] | -1;
    uint16_t targetPort = config["osc"]["targetPort"] | -1;
    String ip = config["osc"]["targetIP"];
    IPAddress targetIP = IPAddress();
    bool broadcast = targetIP == IPAddress();
    long oscPingTimeoutMs = config["osc"]["oscPingTimeoutMs"] | 3000;
    bool oscSendDebug = config["osc"]["oscSendDebug"] | false;
    bool oscReceiveDebug = config["osc"]["oscReceiveDebug"] | false;

    osc = new OSCManager(listeningPort, targetPort, targetIP, broadcast, boardName, oscPingTimeoutMs, oscSendDebug, oscReceiveDebug);
    osc->addListener(std::bind(&WifiModule::gotOSCCommand, this, std::placeholders::_1));
  }

  bool serverDebug = config["webserver"]["serialDebug"] | false;
  configServer = new ConfigWebserver(serverDebug);

  initSTA();
}

void WifiModule::update()
{

  if (millis() % 5000 < 1)
    dbg(String("status: " + String(WiFi.status())));

  switch (WiFi.status())
  {
  case WL_NO_SSID_AVAIL: // 1: after disconnection
    if (millis() - lastDisconnectTime > connectionTimeoutMs)
      initAP();
    break;

  case WL_NO_SHIELD: // 255
                     // AP running
    configServer->update();
    if (millis() - configPortalStartTimeMs > configPortalTimeoutMs) ESP.restart();

  case WL_CONNECTED:
    ArduinoOTA.handle();
    if (osc)
      osc->update();
    break;

  case WL_IDLE_STATUS: // 0: connected but no IP yet
    dbg(".");
    break;

  case WL_DISCONNECTED:
    break;

  case WL_CONNECT_FAILED:
    break;

  case WL_CONNECTION_LOST:
    break;

  default:
    break;
  }
}

void WifiModule::initAP()
{
  dbg("START AP");
  configPortalStartTimeMs = millis();

  if (WiFi.isConnected())
    WiFi.disconnect();

  WiFi.mode(WIFI_AP);
  WiFi.setSleep(false); // can improve ap stability

  String apName = "CONFIG " + boardName;
  WiFi.softAP(apName.c_str());
  initMDNS();
}

void WifiModule::initSTA()
{
  dbg("START STA");

  if (WiFi.isConnected())
    WiFi.disconnect();

  Preferences prefs;
  prefs.begin("wifi");

  String ssid = prefs.getString("ssid", "");
  String pwd = prefs.getString("pwd", "");
  prefs.end();

  if (ssid == "")
  {
    dbg("no ssid stored => start config portal");
    initAP();
  }
  else
  {
    WiFi.mode(WIFI_STA);
    dbg("Connecting to " + ssid + " (" + pwd + ")...");
    WiFi.begin(ssid.c_str(), pwd.c_str());
  }
}

void WifiModule::initMDNS()
{
  WiFi.setHostname(boardName.c_str());

  dbg("creating mDNS instance: " + boardName);
  if (MDNS.begin(boardName.c_str()))
  {
    MDNS.addService("_osc", "_udp", osc->listeningPort);
    MDNS.addServiceTxt("osc", "udp", "boardName", boardName.c_str());

    MDNS.addService("_http", "_tcp", 80);
    dbg("OSC Zeroconf service added sucessfully !");
  }
  else
    err("could not setup MDNS");
}

void WifiModule::initOTA()
{
  dbg("init OTA");
  ArduinoOTA.setHostname(boardName.c_str());
  ArduinoOTA.onStart([]()
                     {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("[OTA] Start updating " + type); });
  ArduinoOTA.onEnd([]()
                   { Serial.println("\nEnd"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        { Serial.printf("Progress: %u%%\n", (progress / (total / 100))); });
  ArduinoOTA.onError([](ota_error_t error)
                     {
    Serial.printf("[OTA] Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    } });
  ArduinoOTA.begin();
}

void WifiModule::gotOSCCommand(const Command &command)
{
  sendEvent(command);
}

void WifiModule::WiFiEvent(WiFiEvent_t event, arduino_event_info_t info)
{
  switch (event)
  {
  case ARDUINO_EVENT_WIFI_STA_START:
    dbg("Event: WiFi client started");
    lastConnectTime = millis();
    lastDisconnectTime = millis();
    break;

  case ARDUINO_EVENT_WIFI_STA_CONNECTED:
    dbg("Event: Connected to access point");
    break;

  case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    dbg("Event: Disconnected from WiFi access point with reason:");
    dbg(String(info.wifi_sta_disconnected.reason));
    // For some reason when unable to connect this event is triggered
    // once with reason 0 then every second with reason 201 WIFI_REASON_NO_AP_FOUND
    // or every second with reason 15 WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT
    switch (info.wifi_sta_disconnected.reason)
    {
    case 15:
      err("incorrect Wifi credentials, closing and start AP...");
      lastDisconnectTime = millis();
      if (osc)
        osc->close();
      MDNS.end();
      initAP();
      // ArduinoOTA.end();
      break;

    case 0:
      dbg("closing...");
      lastDisconnectTime = millis();
      if (osc)
        osc->close();
      MDNS.end();
      ArduinoOTA.end(); // FIXME only if started already
      break;
    }
    break;

  case ARDUINO_EVENT_WIFI_AP_START:
    dbg("Event: WiFi access point started");
    dbg(String(info.wifi_sta_disconnected.reason));
    WiFi.softAPsetHostname(boardName.c_str()); // after we get IP
    configServer->start();
    initMDNS();
    initOTA();
    if (osc)
    {
      osc->open(WiFi.softAPBroadcastIP(), WiFi.softAPIP());
      osc->doBroadcast = true;
    }
    break;

  case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    dbg("Event: Obtained IP address: " + WiFi.localIP().toString());
    initMDNS();
    initOTA();
    if (osc)
    {
      osc->open(WiFi.broadcastIP(), WiFi.gatewayIP());
    }
    // server->start(); TODO ADD WEBSERVER
    break;

  case ARDUINO_EVENT_WIFI_READY:
    dbg("Event: WiFi interface ready");
    break;
  case ARDUINO_EVENT_WIFI_SCAN_DONE:
    dbg("Event: Completed scan for access points");
    break;
  case ARDUINO_EVENT_WIFI_STA_STOP:
    dbg("Event: WiFi clients stopped");
    break;
  case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
    dbg("Event: Authentication mode of access point has changed");
    break;
  case ARDUINO_EVENT_WIFI_STA_LOST_IP:
    dbg("Event: Lost IP address and IP address is reset to 0");
    break;
  case ARDUINO_EVENT_WPS_ER_SUCCESS:
    dbg("Event: WiFi Protected Setup (WPS): succeeded in enrollee mode");
    break;
  case ARDUINO_EVENT_WPS_ER_FAILED:
    dbg("Event: WiFi Protected Setup (WPS): failed in enrollee mode");
    break;
  case ARDUINO_EVENT_WPS_ER_TIMEOUT:
    dbg("Event: WiFi Protected Setup (WPS): timeout in enrollee mode");
    break;
  case ARDUINO_EVENT_WPS_ER_PIN:
    dbg("Event: WiFi Protected Setup (WPS): pin code in enrollee mode");
    break;
  case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
    dbg("Event: Client connected");
    break;
  case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
    dbg("Event: Client disconnected");
    break;

  case ARDUINO_EVENT_WIFI_AP_STOP:
    dbg("Event: WiFi access point  stopped");
    break;
  case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
    dbg("Event: Assigned IP address to client");
    break;
  case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
    dbg("Event: Received probe request");
    break;
  case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
    dbg("Event: AP IPv6 is preferred");
    break;
  case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
    dbg("Event: STA IPv6 is preferred");
    break;
  default:
    break;
  }
}