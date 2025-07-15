# ESPuppet

ESP32 firmware for animatronics and connected stage props

GETTING STARTED
remove ESP32Async and RPAsyncTCP from ESPAsyncWebServer depedencies
which ESP32 platform to use ?

ROADMAP
- color templates for wifidebug
- Parameters -> automated loadConfig + no handleOSCCommand
- FileManager singleton 
- firmware version ?

FIXME filter ping messages, multiport yo
FIXME handle if targetIP not valid

- ledModule advertise/notify -> wifi debug
- ServoPWMShield separate class + servo start
- button events in main.cpp ? or behavior inside Module ?
- pin management -> advertise config errors

TO DIG
- wifi set power
- differences Preferences / LittleFS: memmory size ?
- https://github.com/ayushsharma82/WebSerial
- handle if mDNS failed ?
- not sure if wifi host name is usefull for anything

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
