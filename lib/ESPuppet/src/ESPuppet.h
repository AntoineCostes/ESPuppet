#pragma once
#include "util/Includes.h"
#include "common/FileManager.h"
#include "wifi/WifiModule.h"
#include "ledstrip/LedModule.h"
#include "servo/ServoModule.h"
#include "gpio/GPIOModule.h"

class ESPuppet
{
public:
    ESPuppet();
    ~ESPuppet() {}

    void init(String config = "");
    void update();

    WifiModule* wifiModule = new WifiModule();
    GPIOModule* gpioModule = new GPIOModule();
    LedModule* ledModule = new LedModule();
    ServoModule* servoModule = new ServoModule();

protected:
    void WiFiEvent(WiFiEvent_t event, arduino_event_info_t info);
    void gotOSCCommand(const Command &command);

    std::vector<std::unique_ptr<Module>> modules;

};