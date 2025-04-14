# ESPuppet
ESP32 firmware for animatronics and connected stage props

ROADMAP
- ledModule advertise/notify -> wifi debug
- button events in main.cpp
- Parameters -> automated loadConfig + no handleOSCCommand
- config portal html -> setup wifi + pick config file
- pin management -> config errors

TO DIG
- wifi set power
- differences Preferences / LittleFS: memmory size ?
- https://github.com/ayushsharma82/WebSerial

IDEAS
- print not found host/url on webserver
- variadic sendOSC + include OSC lib
- clarify serialDebug implementation
- Component checkRange

COSMETICS
- char * name
- variadic reservePins
- color instead of r, g, b in LedModule