#include "ESPuppet.h"

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
    servoModule.init();
    gpioModule.init();

    // load config file
    String configFileName = "default";
    Preferences prefs;
    prefs.begin("ESPuppet");
    // prefs.putString("config", "houdini_fire");
    if (prefs.isKey("config")) configFileName = prefs.getString("config");
    else prefs.putString("config", "default");
    // configFileName = prefs.getString("config", "default") same ?
    prefs.end();
    Serial.println("loading config: "+configFileName);
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
          servoModule.loadConfig(json["servo"].as<JsonObject>());
          gpioModule.loadConfig(json["gpio"].as<JsonObject>());
        }
    } else Serial.println("no config file ! Please upload LittleFS image");

    Serial.println("INIT OK");
}

void ESPuppet::update()
{
    ledModule.update();
    wifiModule.update();
    gpioModule.update();
    servoModule.update();
}

void ESPuppet::gotOSCCommand(const Command &command)
{ 
  if (command.targetModule.equals("ledstrip")) ledModule.handleOSCCommand(command.command);
  else if (command.targetModule.equals("gpio")) gpioModule.handleOSCCommand(command.command);
  else if (command.targetModule.equals("servo")) servoModule.handleOSCCommand(command.command);
  else Serial.println("command not delivered to module: "+command.targetModule);
}

// TODO make ledModule Status + notify
void ESPuppet::WiFiEvent(WiFiEvent_t event, arduino_event_info_t info)
{
  Serial.println("EVENT "+String(event));

  switch (event)
  {
  case ARDUINO_EVENT_WIFI_STA_START:
    ledModule.setPattern(LedPattern::OSCILLATOR, 0, 50, 100);
    break;
  case ARDUINO_EVENT_WIFI_STA_STOP:
    ledModule.setPattern(LedPattern::BLINK, 100, 0, 0);
    break;
  case ARDUINO_EVENT_WIFI_STA_CONNECTED:
    ledModule.setPattern(LedPattern::SOLID, 0, 100, 0);
    break;
  case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    ledModule.setPattern(LedPattern::SOLID, 100, 0, 0);
    break;
  case ARDUINO_EVENT_WIFI_AP_START:
    ledModule.setPattern(LedPattern::OSCILLATOR, 0, 100, 100);
    break;
  case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
  // ledModule.setPattern(LedPattern::SOLID, 100, 0, 0);
    // notify client connected
    break;
  default:
    break;
  }
}