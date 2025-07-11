# ESPuppet

ESP32 firmware for animatronics and connected stage props

ROADMAP
OK neopixel gamma
OK OSC broadcast on boot
OK fix crash if no littleFS => c'était le webserver et l'osc qui n'était pas instanciés
OK liste des config files
OK config portal html -> setup wifi + pick config file

FIXME filter ping messages, multiport yo
FIXME handle if targetIP not valid

- upload config file
- list previous wifi credentials
- niceName from config ? est-ce qu'on peut avoir des entrées identiques dans un JSON ?
- color templates for wifidebug
- Parameters -> automated loadConfig + no handleOSCCommand
- FileManager singleton 


- ledModule advertise/notify -> wifi debug
- ServoPWMShield separate class + servo start
- button events in main.cpp
- pin management -> advertise config errors

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
