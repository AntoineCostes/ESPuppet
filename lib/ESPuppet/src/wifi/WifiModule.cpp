#include "WifiModule.h"

WifiModule::WifiModule() : Module("wifi", true), numFailedAttempts(0), onAir(false), osc(nullptr)
{
}

void WifiModule::init()
{
  // TODO declare parameters
  connectionTimeoutMs = 5000;
  configPortalTimeoutMs = 5 * 60 * 1000;
  configServer = new ConfigWebserver(true);
  hasWebServer = true;
  lastConnectTime = millis();

  WiFi.onEvent(std::bind(&WifiModule::WiFiEvent, this, std::placeholders::_1, std::placeholders::_2));

  WiFi.setAutoReconnect(true);
  WiFi.setSleep(false);
}

void WifiModule::loadConfig(JsonObject const &config)
{
  if (config) Serial.println("");
  serialDebug = config["serialDebug"] | serialDebug;
  connectionTimeoutMs = config["connectionTimeoutMs"] | connectionTimeoutMs;
  configPortalTimeoutMs = config["configPortalTimeoutMs"] | configPortalTimeoutMs;
  hasWebServer = config["hasWebServer"] | hasWebServer; // TODO webserverdebug ?

  if (config["osc"])
  {
    uint16_t listeningPort = config["osc"]["listeningPort"] | -1;
    uint16_t targetPort = config["osc"]["targetPort"] | -1;
    String ip = config["osc"]["targetIP"] | "";
    IPAddress targetIP = IPAddress();
    bool broadcast = !targetIP.fromString(ip);
    long oscPingTimeoutMs = config["osc"]["oscPingTimeoutMs"] | 3000;
    bool oscSendDebug = config["osc"]["oscSendDebug"] | false;
    bool oscReceiveDebug = config["osc"]["oscReceiveDebug"] | false;

    osc = new OSCManager(listeningPort, targetPort, targetIP, broadcast, oscPingTimeoutMs, oscSendDebug, oscReceiveDebug);
    osc->addListener(std::bind(&WifiModule::gotOSCCommand, this, std::placeholders::_1));
  }

  initSTA();
}

void WifiModule::update()
{
  switch (WiFi.status())
  {
    // I don't understand when it gets in WL_NO_SSID_AVAIL and when it gets in WL_DISCONNECTED,
    // it got to both in a row without environmental change (no router)
  case WL_NO_SSID_AVAIL:
    if (millis() % 1000 < 1) dbg("STATUS: ssid not valid");

    if (millis() - lastConnectTime > connectionTimeoutMs)
    {
      dbg("we got disconnected for a while, disconnect and start AP");
      disconnect();
      initAP();
    }
    break;

  case WL_DISCONNECTED:
    if (millis() % 3000 < 1) dbg("STATUS: disconnected");
    if (millis() - lastConnectTime > connectionTimeoutMs)
    {
      dbg("we could not connect for a while, try again");
      disconnect();
      initSTA();
    }
    break;

  case WL_CONNECT_FAILED:
    if (millis() % 1000 < 1) dbg("STATUS: failed to connect");

    if (millis() - lastConnectTime > connectionTimeoutMs)
    {
      // dbg("connection failed, disconnect and start AP");
      // disconnect();
      // initAP();
      dbg("connection failed, try again");
      disconnect();
      delay(1000);
      initSTA();
    }
    break;

  case WL_NO_SHIELD: // 255
                     // AP running
    if (millis() % 5000 < 1) dbg("STATUS: AP running");
    if (millis() - configPortalStartTimeMs > configPortalTimeoutMs)
    {
      log("PORTAL TIMEOUT EXPIRED - RESTART");
      ESP.restart();
    }
  
    // I don't know status does not switch to WL_CONNECTED when AP is running
  case WL_CONNECTED:
    if (millis() % 5000 < 1 && WiFi.status() != WL_NO_SHIELD) dbg("STATUS: CONNECTED TO STA");
    if (onAir)
    {
      ArduinoOTA.handle();
      if (hasWebServer) configServer->update();
      if (osc) osc->update();
    }
    break;

  case WL_IDLE_STATUS: // 0: connected but no IP yet
    if (millis() % 1000 < 1) dbg("STATUS: waiting for ip");
    break;


  case WL_CONNECTION_LOST:
    if (millis() % 1000 < 1) dbg("STATUS: connection lost");
    break;

  default:
    break;
  }
}

void WifiModule::initAP()
{
  String apName = "CONFIG " + FileManager::getCurrentConfigNiceName();

  dbg("\nSTART AP: " + apName);
  configPortalStartTimeMs = millis();
  lastConnectTime = millis();

  if (WiFi.isConnected())  WiFi.disconnect();

  WiFi.mode(WIFI_AP);
  WiFi.setSleep(false); // can improve ap stability
  WiFi.softAP(apName.c_str());

  WiFi.setTxPower(WIFI_POWER_8_5dBm); // magic number
}

void WifiModule::initSTA()
{
    if (numFailedAttempts > 3) 
    {
      log("got disconneged too many times, start AP instead of STA");
      initAP();
      return;
    }

  dbg("START STA");
  lastConnectTime = millis();

  if (WiFi.isConnected()) WiFi.disconnect();

  String ssid = FileManager::currentSSID();
  String pwd = FileManager::currentPwd();

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
    WiFi.setTxPower(WIFI_POWER_8_5dBm);
  }
}

void WifiModule::initZeroConf()
{
  // WiFi.setHostname(FileManager::getCurrentConfigNiceName().c_str());
  ArduinoOTA.setHostname(FileManager::getCurrentConfigName().c_str());

  ArduinoOTA.onStart([]()
                     {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)  type = "sketch";
    else type = "filesystem"; // U_FS

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("[OTA] Start updating " + type); });
  ArduinoOTA.onEnd([]()
                   { Serial.println("\nEnd"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        { Serial.printf("Progress: %u%%\n", (progress / (total / 100))); });
  ArduinoOTA.onError([](ota_error_t error)
                     {
    Serial.printf("[OTA] Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

  if (osc)
  {
    if (MDNS.addService("_osc", "_udp", osc->listeningPort))
    {
      dbg("OSC Zeroconf service added sucessfully !");
      MDNS.addServiceTxt("osc", "udp", "board", ARDUINO_BOARD);
      MDNS.addServiceTxt("osc", "udp", "config", FileManager::getCurrentConfigName().c_str());
    }
    else
    {
      err("OSC zeroconf services could not be added");
      log(String(osc->listeningPort));
      log(FileManager::getCurrentConfigName());
    }
  }

  if (MDNS.addService("_http", "_tcp", 80)) dbg("TCP service added sucessfully !");
  else err("TCP service could not be added");
}

void WifiModule::disconnect()
{
  dbg("\t === DISCONNECT ===");
  onAir = false;
  if (osc) osc->close();
  if (hasWebServer) configServer->stop();
  ArduinoOTA.end();
  numFailedAttempts++;
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
    dbg("Event: Start connecting to router");
  tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, FileManager::getCurrentConfigNiceName().c_str());
    break;

  case ARDUINO_EVENT_WIFI_STA_CONNECTED:
    dbg("Event: Connected to router");
    break;

  case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    dbg("Event: Disconnected from WiFi access point with reason: " + String(info.wifi_sta_disconnected.reason));
    // For some reason when unable to connect this event is triggered
    // once with reason 0 then every second with reason 201 WIFI_REASON_NO_AP_FOUND
    // or every second with reason 15 WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT

    switch (info.wifi_sta_disconnected.reason)
    {
      // could not connect to STA
    case WIFI_REASON_AUTH_EXPIRE: // 2
      err("WIFI_REASON_AUTH_EXPIRE");
      break;

    case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT: // 15
      err("handshake timeout expired. incorrect Wifi credentials ?");
      break;

    case WIFI_REASON_TIMEOUT: // 39
      err("WIFI_REASON_TIMEOUT");
      break;

    case 0:
      err("NO REASON, fuck you");
      break;

    case WIFI_REASON_BEACON_TIMEOUT: // 200
      err("beacon timeout. list router ?");
      break;

    case WIFI_REASON_NO_AP_FOUND: // 201
      err("no AP found");
      break;
    }
    break;

  case ARDUINO_EVENT_WIFI_AP_START:
    dbg("Event: WiFi access point started");
    
    WiFi.softAPsetHostname(FileManager::getCurrentConfigNiceName().c_str());
    initZeroConf();
    if (osc)
    {
      osc->open(WiFi.softAPBroadcastIP(), WiFi.softAPIP());
      osc->doBroadcast = true;
    }
    if (hasWebServer) configServer->start();
    numFailedAttempts = 0;
    onAir = true;
    break;

  case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    dbg("Event: Obtained IP address: " + WiFi.localIP().toString());
    initZeroConf();
    if (osc) osc->open(WiFi.broadcastIP(), WiFi.gatewayIP());
    if (hasWebServer) configServer->start();
    onAir = true;
    numFailedAttempts = 0;
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
    err("unknown Wifi event !");
    break;
  }
}