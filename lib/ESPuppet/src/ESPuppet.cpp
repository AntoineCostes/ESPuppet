#include "ESPuppet.h"

ESPuppet::ESPuppet()
{
}

void ESPuppet::init()
{
    FileManager::init();

    // init each module
    WiFi.onEvent(std::bind(&ESPuppet::WiFiEvent,this,std::placeholders::_1,std::placeholders::_2));
    wifiModule.init();
    wifiModule.addListener(std::bind(&ESPuppet::gotOSCCommand, this, std::placeholders::_1));
    ledModule.init();
    servoModule.init();
    gpioModule.init();

    Serial.println("");
    Serial.println("");

    File config = FileManager::openConfigFile();
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
      Serial.println("INIT OK");
      Serial.println("");
      Serial.println("");
      
      FileManager::printWifiCredentials();

    // Preferences prefs;
    // prefs.begin("wifi_creds");
    // prefs.clear();
    // prefs.end();

      // FileManager::registerWifiCredentials("under the sunshine", "bibimbap");
      // FileManager::registerWifiCredentials("akindofmagic", "H0udini25");
      // FileManager::registerWifiCredentials("LeNet", "connectemoi");
    } else 
    {
      Serial.println("no config file ! Please upload LittleFS image");
      // TODO advertise error with leds
      wifiModule.initAP();
    }
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
  Serial.println("WIFI EVENT "+String(event));

  switch (event)
  {
  case ARDUINO_EVENT_WIFI_STA_START:
    ledModule.setPattern(LedPattern::OSCILLATOR, 0, 50, 100, 1.0f, 0.2f);
    break;
  case ARDUINO_EVENT_WIFI_STA_STOP:
    ledModule.setPattern(LedPattern::BLINK, 100, 0, 0, 1.0f, 0.2f);
    break;
  case ARDUINO_EVENT_WIFI_STA_CONNECTED:
    ledModule.setPattern(LedPattern::SOLID, 0, 100, 0, 1.0f, 0.2f);
    break;
  case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    ledModule.setPattern(LedPattern::SOLID, 100, 0, 0, 1.0f, 0.2f);
    break;

  case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
    ledModule.setPattern(LedPattern::BLINK, 100, 0, 0, 1.0f, 0.2f);
    break;

  case ARDUINO_EVENT_WIFI_AP_START:
    ledModule.setPattern(LedPattern::OSCILLATOR, 0, 100, 100, 1.0f, 0.2f);
    break;
  case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
  // ledModule.setPattern(LedPattern::SOLID, 100, 0, 0);
    // TODO notify client connected ?
    break;
  default:
    break;
  }
}