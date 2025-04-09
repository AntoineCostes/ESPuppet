#pragma once
#include "common/Includes.h"
#include "files/FileManager.h"
#include "wifi/WifiModule.h"
#include "outputs/ledstrip/LedModule.h"

class ESPuppet
{
public:
    ESPuppet();
    ~ESPuppet() {}

    static String niceName;

    void init();
    void update();

    void initWifi();

    FileManager fileModule;
    LedModule ledModule;
    WifiModule wifiModule;

protected:
    void WiFiEvent(WiFiEvent_t event, arduino_event_info_t info);
};