# ESPuppet

ESP32 firmware for animatronics and connected stage props

ROADMAP
- liste des config files
- fix crash if no littleFS
- ServoPWMShield separate class + servo start
OK neopixel gamma
OK OSC broadcast on boot
FIXME filter ping messages, multiport yo
- ledModule advertise/notify -> wifi debug
- button events in main.cpp
- Parameters -> automated loadConfig + no handleOSCCommand
- config portal html -> setup wifi + pick config file
- pin management -> advertise config errors
- niceName from config ? est-ce qu'on peut avoir des entrées identiques dans un JSON ?
- handle if targetIP not valid

TO DIG
- wifi set power
- differences Preferences / LittleFS: memmory size ?
- https://github.com/ayushsharma82/WebSerial
- handle if mDNS & OTA failed

IDEAS

- lastUpdateMs in component ?
- inputComponent génère des évènement, outputComponent est contrôlable par OSC ?
- FileManager Component
- print not found host/url on webserver
- variadic sendOSC + include OSC lib
- clarify serialDebug implementation
- Component checkRange

COSMETICS

- char \* name
- variadic reservePins
- color instead of r, g, b in LedModule

GETTING STARTED
remove ESP32Async and RPAsyncTCP from ESPAsyncWebServer depedencies
which ESP32 platform to use ?
