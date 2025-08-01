# ESPuppet

ESP32 firmware for animatronics and connected stage props

GETTING STARTED
remove ESP32Async and RPAsyncTCP from ESPAsyncWebServer depedencies
which ESP32 platform to use ?

ROADMAP
FIXME handle if targetIP not valid
FIXME servo default values

OK local IP on webserver index + select current config 
OK firmware version ? => compiled date instead

- odrive module + optional server (or debug wroom server)
- ledModule advertise/notify -> wifi debug
- ServoPWMShield separate class + servo start
- button events in main.cpp ? or behavior inside Module ?
- pin management -> advertise config errors
- Parameters -> automated loadConfig + no handleOSCCommand: faut clarifier comment faire le callback


TO DIG
OK wifi set power
- differences Preferences / LittleFS: memmory size ?
- https://github.com/ayushsharma82/WebSerial
- handle if mDNS failed ?
- not sure if wifi host name is usefull for anything

IDEAS
- allowed pins for each module and boards ?
- color templates for wifidebug (parameters)
- lastUpdateMs in component ?
- inputComponent génère des évènement, outputComponent est contrôlable par OSC ?
- FileManager Component ?
- print not found host/url on webserver
- variadic sendOSC + include OSC lib
- clarify serialDebug implementation

COSMETICS
- timer with callback
- FileManager singleton 
- char \* name
- variadic reservePins
- color instead of r, g, b in LedModule
