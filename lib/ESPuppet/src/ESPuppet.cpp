#include "ESPuppet.h"

// ROADMAP
// OSC led control => OSCCommand + dispatch to modules
// led wifidebug => ledModule advertise/notify
// events in main.cpp => buttonEvent
// Configportal
// pick possible configs in portal

// Gestion des pin
// Wifi set power ?
// clarifier où est le serialDebug

// IDEAS
// Component checkRange

// COSMETICS
// OSCMessage getAddress();
// char * name
// variadic reservePins


ESPuppet::ESPuppet()
{
}

String ESPuppet::niceName = "ESPuppet";

void ESPuppet::init()
{
    // init each module
    fileModule.init();
    WiFi.onEvent(std::bind(&ESPuppet::WiFiEvent,this,std::placeholders::_1,std::placeholders::_2));
    wifiModule.init();
    wifiModule.addListener(std::bind(&ESPuppet::gotOSCCommand, this, std::placeholders::_1));
    ledModule.init();

    // load config file
    String configFileName = "default";
    Preferences prefs;
    prefs.begin("ESPuppet");
    if (prefs.isKey("config")) configFileName = prefs.getString("config");
    else prefs.putString("config", "default");
    prefs.end();

    configFileName += ".json";
    String filePath = String(ARDUINO_BOARD) + "/"+ configFileName;

    Serial.println("loading config file: " +filePath);

    File config = fileModule.openFile(filePath);
    if (config)
    {
        JsonDocument json;
        DeserializationError error = deserializeJson(json, config);
        if (error)
        {
            Serial.println("failed to deserialize json config");
            Serial.println(error.c_str());
        } else
        {
          ledModule.loadConfig(json["leds"].as<JsonObject>());
          wifiModule.loadConfig(json["wifi"].as<JsonObject>());
        }
    } else Serial.println("no config file ! Please upload LittleFS image");

    Serial.println("INIT OK");
}

void ESPuppet::update()
{
    ledModule.update();
    wifiModule.update();
}

void ESPuppet::gotOSCCommand(const Command &command)
{ 
  // TODO loop over modules
  Serial.println("command for: "+command.target);
  if (command.target.equals("led")) Serial.println("yes");
  if (command.target.equals("led")) ledModule.handleCommand(command);
}

// TODO make ledModule Status + notify
void ESPuppet::WiFiEvent(WiFiEvent_t event, arduino_event_info_t info)
{
  switch (event)
  {
  case ARDUINO_EVENT_WIFI_STA_START:
    ledModule.setWave(0, 0, 0, 100);
    break;
  case ARDUINO_EVENT_WIFI_STA_STOP:
    ledModule.setBlink(0, 100, 0, 0);
    break;
  case ARDUINO_EVENT_WIFI_STA_CONNECTED:
    ledModule.setSolid(0, 0, 100, 0);
    break;
  case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    ledModule.setSolid(0, 100, 0, 0);
    break;
  case ARDUINO_EVENT_WIFI_AP_START:
    ledModule.setWave(0, 0, 100, 100);
    break;
  case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
    // ledModule.setSolid(0, 100, 0, 0);
    // notify client connected
    break;
  default:
    break;
  }
}