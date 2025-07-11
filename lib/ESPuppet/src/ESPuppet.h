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

    static String niceName;

    void init();
    void update();

    void initWifi();

    // FileManager fileModule;
    WifiModule wifiModule;
    GPIOModule gpioModule;
    LedModule ledModule;
    ServoModule servoModule;

protected:
    void WiFiEvent(WiFiEvent_t event, arduino_event_info_t info);
    void gotOSCCommand(const Command &command);

};