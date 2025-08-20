#include "WifiModule.h"

WifiModule::WifiModule() : Module("wifi", true), 
osc(nullptr), connectionAttempts(0), numDisconnections(0), 
portalTimeout(5 * 60 * 1000), // 5 mn
disconnectedTimeout(15*1000), // 15 sec
onAir(false), isConnecting(false)
{
}

void WifiModule::init()
{
  // TODO declare parameters
  configServer = new ConfigWebserver(true);
  hasWebServer = true;

  portalTimeout.setCallback(std::bind(&WifiModule::onAPForTooLong, this));
  disconnectedTimeout.setCallback(std::bind(&WifiModule::disconnectedForTooLong, this));

  WiFi.onEvent(std::bind(&WifiModule::WiFiEvent, this, std::placeholders::_1, std::placeholders::_2));

  WiFi.setAutoReconnect(true);
  WiFi.setSleep(false);
}

void WifiModule::disconnectedForTooLong()
{
  log("DISCONNECTION TIMEOUT EXPIRED - RESTART");
  log("");
  log("");
  ESP.restart();
}

void WifiModule::onAPForTooLong()
{
  log("CONFIG PORTAL TIMEOUT EXPIRED - RESTART");
  log("");
  log("");
  ESP.restart();
}

void WifiModule::loadConfig(JsonObject const &config)
{
  if (config) Serial.println("");
  serialDebug = config["serialDebug"] | serialDebug;
  hasWebServer = config["hasWebServer"] | hasWebServer; // TODO webserverdebug ?
  if (config["configPortalTimeoutMs"]) portalTimeout.set(config["configPortalTimeoutMs"]);
  if (config["disconnectedTimeoutMs"]) disconnectedTimeout.set(config["disconnectedTimeoutMs"]);

  if (config["osc"])
  {
    uint16_t listeningPort = config["osc"]["listeningPort"] | -1;
    uint16_t targetPort = config["osc"]["targetPort"] | -1;
    String ip = config["osc"]["targetIP"] | ""; // FIXME cause of udp could not send data ?
    IPAddress targetIP = IPAddress();
    bool broadcast = !targetIP.fromString(ip);
    long oscPingTimeoutMs = config["osc"]["oscPingTimeoutMs"] | 3000;
    bool oscSendDebug = config["osc"]["oscSendDebug"] | false;
    bool oscReceiveDebug = config["osc"]["oscReceiveDebug"] | false;

    log("Register OSC manager");
    osc = new OSCManager(listeningPort, targetPort, targetIP, broadcast, oscPingTimeoutMs, oscSendDebug, oscReceiveDebug);
    osc->addListener(std::bind(&WifiModule::gotOSCCommand, this, std::placeholders::_1));
  }

  initSTA();
}


void WifiModule::update()
{
  switch (WiFi.status())
  {
    // AP running
    case WL_NO_SHIELD: // 255
      portalTimeout.update();

      // I don't know status does not switch to WL_CONNECTED when AP is running
    case WL_CONNECTED: // 3
      if (disconnectedTimeout.isRunning) 
      {
         // FIXME why is it not stopped already ?
        Serial.println("CONNECTED > STOP TIMEOUT");
        disconnectedTimeout.stop();
      }
      if (millis() % 5000 < 1)
      {
        if (WiFi.status() == WL_CONNECTED) dbg("- CONNECTED TO STA");
        if (WiFi.status() == WL_NO_SHIELD) dbg("- AP RUNNING");
      } 
      if (onAir)
      {
        ArduinoOTA.handle();
        if (hasWebServer) configServer->update();
        if (osc) osc->update();
      }
      break;

    case WL_NO_SSID_AVAIL: // 1
      // Failed to connect
    case WL_CONNECT_FAILED: // 4
    case WL_DISCONNECTED: // 6
      if (millis() % 2000 < 1) dbg("- DISCONNECTED");
      if (!disconnectedTimeout.isRunning) disconnectedTimeout.start(); // auto launch if disconnection event was not caught
      disconnectedTimeout.update();
      break;

    case WL_IDLE_STATUS: // 0
      if (millis() % 2000 < 1)  dbg("- WAITING FOR IP");
      if (!disconnectedTimeout.isRunning) disconnectedTimeout.start(); // auto launch if disconnection event was not caught
      disconnectedTimeout.update();
      break;

    default:
      if (millis() % 1000 < 1) err("- UNKWOWN STATUS: "+String(WiFi.status()));
      break;
  }
}

void WifiModule::goOnAir()
{
  if (!onAir)
  {
    dbg("\t === ON AIR ===");
    onAir = true;
    initZeroConf();
    if (osc)
    {
      osc->open();
      if (isAP()) osc->doBroadcast = true;
    }
    if (hasWebServer) configServer->start();
  }
}

void WifiModule::goOffAir()
{
  if (onAir)
  {
    dbg("\t === OFF AIR ===");
    onAir = false;
    if (osc) osc->close();
    if (hasWebServer) configServer->stop();
    ArduinoOTA.end();
  } 
}

void WifiModule::initAP()
{
  if (isConnecting) err("is already connecting, pass");
  if (isConnecting) return;
  isConnecting = true;

  String apName = "CONFIG " + FileManager::getCurrentConfigNiceName();
  dbg("\n=== START AP: " + apName);
  goOffAir();

  if (WiFi.isConnected())  WiFi.disconnect();

  WiFi.mode(WIFI_AP);
  WiFi.setSleep(false); // can improve ap stability
  WiFi.softAP(apName.c_str());

  WiFi.setTxPower(WIFI_POWER_8_5dBm); // magic number
  isConnecting = false;
}

void WifiModule::initSTA()
{
  connectionAttempts++;
  if (connectionAttempts > 3) 
  {
    log("we tried 3 times already, start AP instead of STA");
    initAP();
    return;
  } 

  if (isConnecting) err("is already connecting, pass");
  if (isConnecting) return;
  isConnecting = true;

  goOffAir();
  dbg("START STA");

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
  isConnecting = false;
}

void WifiModule::initZeroConf()
{
  // this also sets the hostname for zeroconf
  ArduinoOTA.setHostname((FileManager::getCurrentConfigName() + (isAP()?"_AP":"")).c_str());

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
    }
  }

  if (MDNS.addService("_http", "_tcp", 80)) dbg("TCP service added sucessfully !");
  else err("TCP service could not be added");
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
    // when does that happen ?
    dbg("Event: Start connecting to router");
    break;

  case ARDUINO_EVENT_WIFI_STA_CONNECTED:
    dbg("Event: Connected to router");
    tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, FileManager::getCurrentConfigNiceName().c_str());
    connectionAttempts = 0;
    numDisconnections = 0;
    portalTimeout.stop();
    disconnectedTimeout.stop();
    break;

  case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    dbg("Event: Obtained IP address: " + WiFi.localIP().toString());
    goOnAir();
    break;

  case ARDUINO_EVENT_WIFI_STA_STOP:
    dbg("Event: WiFi clients stopped");
    break;

  case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    // For some reason when unable to connect this event is triggered
    // once with reason 0 then every second with reason 201 WIFI_REASON_NO_AP_FOUND
    // or every second with reason 15 WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT

    switch (info.wifi_sta_disconnected.reason)
    {
      // could not connect to STA
    case WIFI_REASON_AUTH_EXPIRE: // 2
    // may happen just before connecting to router
      dbg("STA_DISCONNECTED Event: WIFI_REASON_AUTH_EXPIRE");
      break;

    case 0:
      dbg("STA_DISCONNECTED Event: NO REASON, fuck you");
    case WIFI_REASON_NOT_AUTHED: // 6 could not reproduce yet
    case WIFI_REASON_NOT_ASSOCED: // 7 lost connection to STA
    case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT: // 15 can happen with incorrect credentials
    case WIFI_REASON_TIMEOUT: // 39 could not reproduce yet
    case WIFI_REASON_BEACON_TIMEOUT: // 200 could not reproduce yet
    case WIFI_REASON_AUTH_FAIL: // 201 could not find network or wrong password
    // 51
      if (info.wifi_sta_disconnected.reason == WIFI_REASON_NOT_AUTHED) dbg("= Connection lost !");
      if (info.wifi_sta_disconnected.reason == WIFI_REASON_NOT_ASSOCED) dbg("= Connection lost !");
      if (info.wifi_sta_disconnected.reason == WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT) err("=== HANDSHAKE_TIMEOUT");
      if (info.wifi_sta_disconnected.reason == WIFI_REASON_TIMEOUT) err("=== TIMEOUT");
      if (info.wifi_sta_disconnected.reason == WIFI_REASON_BEACON_TIMEOUT) err("=== BEACON TIMEOUT");
      if (info.wifi_sta_disconnected.reason == WIFI_REASON_AUTH_FAIL) dbg("= Could not connect");
      WiFi.disconnect();
      initSTA();
      if (!disconnectedTimeout.isRunning)  disconnectedTimeout.start();
      break;

    default:
      if (info.wifi_sta_disconnected.reason == WIFI_REASON_NOT_AUTHORIZED_THIS_LOCATION) // 30
        err("= location not authorized ");
      else if (info.wifi_sta_disconnected.reason == WIFI_REASON_NO_AP_FOUND) // 201
        // happens every 2sec when the SSID is not visible
        dbg("= Failed to reach "+FileManager::currentSSID());
      else
        dbg("STA_DISCONNECTED Event: unknown reason: " + String(info.wifi_sta_disconnected.reason));
        
      numDisconnections++;
      if (numDisconnections > 5) initAP(); // 5 attempts ~ 10 seconds
      break;
    }
    break;

  case ARDUINO_EVENT_WIFI_AP_START:
    dbg("Event: WiFi access point started");
    WiFi.softAPsetHostname(FileManager::getCurrentConfigNiceName().c_str());
    connectionAttempts = 0;
    numDisconnections = 0;
    disconnectedTimeout.stop();
    portalTimeout.start();
    goOnAir();
    break;

  case ARDUINO_EVENT_WIFI_AP_STOP:
    // This actually never happens, we reboot to exit AP
    dbg("Event: WiFi access point stopped");
    goOffAir();
    break;

  case ARDUINO_EVENT_WIFI_READY:
    dbg("Event: WiFi interface ready");
    break;
  case ARDUINO_EVENT_WIFI_SCAN_DONE:
    dbg("Event: Completed scan for access points");
    break;
  case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
    dbg("Event: Authentication mode of access point has changed");
    break;
  case ARDUINO_EVENT_WIFI_STA_LOST_IP:
    // this is not relevant if we are in AP mode
    if (!isAP())
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

bool WifiModule::isAP()
{
  return WiFi.getMode() == WIFI_MODE_AP;
}