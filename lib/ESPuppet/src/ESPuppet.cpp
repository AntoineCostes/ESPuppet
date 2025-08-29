#include "ESPuppet.h"

ESPuppet::ESPuppet()
{
  modules.emplace_back(wifiModule);
  modules.emplace_back(ledModule);
  modules.emplace_back(servoModule);
  modules.emplace_back(gpioModule);
}

void ESPuppet::init(String config)
{
  FileManager::init();

  if (!config.isEmpty())
    FileManager::setNewConfig(config);


  // default credentials
  if (FileManager::currentSSID().equals(""))
  {
      FileManager::registerWifiCredentials("under the sunshine", "bibimbap");
      FileManager::registerWifiCredentials("akindofmagic", "H0udini25");
      FileManager::registerWifiCredentials("LeNet", "connectemoi");
      FileManager::setWifiCredentials("akindofmagic");
  }
  
  // init wifi module
  WiFi.onEvent(std::bind(&ESPuppet::WiFiEvent, this, std::placeholders::_1, std::placeholders::_2));
  for (auto const &module : modules)
    module.get()->init();
  wifiModule->addListener(std::bind(&ESPuppet::gotOSCCommand, this, std::placeholders::_1));

  Serial.println("");
  Serial.println("");

  // try to load config file from filesytem image, and configure modules according to it
  File configFile = FileManager::openConfigFile();
  if (!configFile)
  {
    Serial.println("no config file ! Please upload LittleFS image");

    // configure default neopixel for wifi debug
    if (String(ARDUINO_BOARD).equals("ESP32C3-SuperMini")) ledModule->registerLedStrip(8, 20, 0.6);
    else if (String(ARDUINO_BOARD).equals("Seeed Studio XIAO ESP32C3")) ledModule->registerLedStrip(10, 20, 0.2);
    
    wifiModule->initAP();
  } else
  {
    JsonDocument json;
    DeserializationError error = deserializeJson(json, configFile);
    if (error)
      Serial.println("failed to deserialize json config : " + String(error.c_str()));
    else
      for (auto const &module : modules)
        module.get()->loadConfig(json[module.get()->name].as<JsonObject>());

    FileManager::printWifiCredentials();
    Serial.println("");
    Serial.println("INIT OK: "+FileManager::getCurrentConfigNiceName());
    Serial.println("");
    Serial.println("");
    
    wifiModule->initSTA();
  }
}

void ESPuppet::update()
{
  for (auto const &module : modules)
    module->update();
}

void ESPuppet::gotOSCCommand(const Command &command)
{
  if (command.command->match("UDP_FAILED"))
  {
    Serial.println("UDP PACKET ERROR");
    ledModule->setPattern(LedPattern::SOLID, 100, 20, 0, 1.0f, 1.0f, 0.2f);
    return;
  }

  bool delivered = false;
  for (auto const &module : modules)
    if (command.targetModule.equals(module->name))
    {
      module.get()->handleOSCCommand(command.command);
      delivered = true;
    }

  if (!delivered)
    Serial.println("ERROR OSC command not delivered to module: " + command.targetModule);
}

// TODO make ledModule Status + notify
void ESPuppet::WiFiEvent(WiFiEvent_t event, arduino_event_info_t info)
{
  // Serial.println("\t WIFI EVENT "+String(event));

  switch (event)
  {
  case ARDUINO_EVENT_WIFI_STA_START:
    ledModule->setPattern(LedPattern::OSCILLATOR, 0, 50, 100, 1.0f, 1.0f, 0.2f);
    break;
  case ARDUINO_EVENT_WIFI_STA_STOP:
    ledModule->setPattern(LedPattern::BLINK, 100, 0, 0, 1.0f, 1.0f, 0.2f);
    break;
  case ARDUINO_EVENT_WIFI_STA_CONNECTED:
    ledModule->setPattern(LedPattern::OSCILLATOR, 0, 50, 100, 2.0f,1.0f,  0.2f);
    break;
  case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    ledModule->setPattern(LedPattern::SOLID, 0, 100, 0, 1.0f, 1.0f, 0.2f);
    break;
  case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    ledModule->setPattern(LedPattern::SOLID, 100, 0, 0, 1.0f, 1.0f, 0.2f);
    break;

  case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
    ledModule->setPattern(LedPattern::BLINK, 100, 0, 0, 1.0f, 1.0f, 0.2f);
    break;

  case ARDUINO_EVENT_WIFI_AP_START:
    ledModule->setPattern(LedPattern::OSCILLATOR, 0, 100, 100, 1.0f, 1.0f, 0.2f);
    break;

  case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
    // ledModule->setPattern(LedPattern::SOLID, 100, 0, 0);
    // TODO notify client connected ?
    break;
  default:
    break;
  }
}