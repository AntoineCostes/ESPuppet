#include "WifiModule.h"

WifiModule::WifiModule() : Module("wifi")
{
}

void WifiModule::init()
{
  // TODO declare parameters
  serialDebug = true;
  connectionTimeoutMs = 10000;

  lastConnectTime = millis();
  lastDisconnectTime = millis();

  WiFi.onEvent(std::bind(&WifiModule::WiFiEvent, this, std::placeholders::_1, std::placeholders::_2));

  WiFi.setAutoReconnect(true);
  // WiFi.setSleep(false);
  // WiFi.setTxPower(WIFI_POWER_19dBm); TODO parameter

  // char test[8] = "1234678";
  // WiFiManagerParameter testParam("server", "mqtt server", test, 8);
  // wm.addParameter(&testParam);
  // WiFiManagerParameter custom_field("customfieldid", "Custom Field Label", "Custom Field Value", 40,"placeholder=\"Custom Field Placeholder\"");
  // wm.addParameter(&custom_field);
  // int customFieldLength = 40;
  // new (&custom_field) WiFiManagerParameter("customfieldid", "Custom Field Label", "Custom Field Value", customFieldLength,"placeholder=\"Custom Field Placeholder\"");
  // wm.addParameter(&custom_field);
  // wm.setSaveParamsCallback(std::bind(&WifiModule::saveParamCallback, this, std::placeholders::_1));

  // wm.setConfigPortalBlocking(false);
  // std::vector<const char *> menu = {"wifinoscan", "info", "param", "restart"};
  // wm.setMenu(menu);
  // wm.setClass("invert");
  // wm.setConnectTimeout(5);       // how long to try to connect for before continuing
  // wm.setConfigPortalTimeout(30); // auto close configportal after n seconds
  // wm.setAPClientCheck(true);     // avoid timeout if client connected to softap
}

void WifiModule::loadConfig(JsonObject const &config)
{
  serialDebug = config["serialDebug"] | serialDebug;
  connectionTimeoutMs = config["connectionTimeoutMs"] | connectionTimeoutMs;

  String configFileName = "default";
  boardName = config["boardName"] | configFileName;

  if (config["osc"])
  {
    uint16_t listeningPort = config["osc"]["listeningPort"] | -1;
    uint16_t targetPort = config["osc"]["targetPort"] | -1;
    String ip = config["osc"]["targetIP"];
    IPAddress targetIP = IPAddress((char *)ip.c_str()) | IPAddress();
    long oscPingTimeoutMs = config["osc"]["oscPingTimeoutMs"] | 3000;
    bool oscSendDebug = config["osc"]["oscSendDebug"] | false;
    bool oscReceiveDebug = config["osc"]["oscReceiveDebug"] | false;

    osc = new OSCManager(listeningPort, targetPort, targetIP, boardName, oscPingTimeoutMs, oscSendDebug, oscReceiveDebug);
    osc->addListener(std::bind(&WifiModule::gotOSCCommand, this, std::placeholders::_1));
  }
  
  bool serverDebug = config["webserver"]["serialDebug"] | false;
  configServer = new ConfigWebserver(serverDebug);

  initSTA();
}

String WifiModule::getDefaultBoardName()
{
  Preferences prefs;
  prefs.begin("ESPuppet");
  String configFileName = prefs.getString("config", "default");
  prefs.end();
  return "ESPuppet-" + configFileName;
}

void WifiModule::update()
{
  if (osc)
    osc->update(); // TODO add OscMgr to props

  if (millis() % 3000 < 1)
    dbg(String(WiFi.status()));

  switch (WiFi.status())
  {
  case WL_NO_SSID_AVAIL: // after disconnection
    if (millis() - lastDisconnectTime > connectionTimeoutMs)
      initAP();
    break;

  case WL_NO_SHIELD: // AP on ?
    // ArduinoOTA.handle();
    configServer->update();
    break;

  case WL_IDLE_STATUS: // connected but no IP yet
    break;

  case WL_DISCONNECTED:
    break;

  case WL_CONNECTED:
    // ArduinoOTA.handle();
    break;

  case WL_CONNECT_FAILED:
    break;

  case WL_CONNECTION_LOST:
    break;

  case WL_STOPPED:// AP on ?
    // ArduinoOTA.handle();
    configServer->update();
    break;

  default:
    break;
  }
}

void WifiModule::initAP()
{
  if (WiFi.isConnected())
    WiFi.disconnect();
  dbg("START AP");

  WiFi.mode(WIFI_AP);
  WiFi.setSleep(false); // can improve ap stability

  // startAP();
  String apName = "CONFIG-" + String(ARDUINO_BOARD);
  WiFi.softAP(apName.c_str());

  // delay(500); 
  // WiFi.softAPsetHostname(boardName.c_str()); // after we get IP
  // server-> start();

  // setupHTTPServer();
  // setupDNSD(); 
  // server-> start
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
    
    initMDNS(); 
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
    if (osc)
      osc->open();
    break;

  case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    dbg("Event: Disconnected from WiFi access point");
    // For some reason when unable to connect this event is triggered
    // once with reason 0 then every second with reason 201 WIFI_REASON_NO_AP_FOUND
    if (info.wifi_sta_disconnected.reason == 0)
    {
      lastDisconnectTime = millis();
      if (osc)
        osc->close();
      dbg("END MDNS & server");
      MDNS.end();
      configServer->stop();
    }
    break;

  case ARDUINO_EVENT_WIFI_AP_START:
    dbg("Event: WiFi access point started");
    dbg(String(info.wifi_sta_disconnected.reason));
    WiFi.softAPsetHostname(boardName.c_str()); // after we get IP
    configServer-> start();
    break;

  case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    Serial.print("Event: Obtained IP address: ");
    Serial.println(WiFi.localIP());
    dbg(String(info.wifi_sta_disconnected.reason));
    // server->start();
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