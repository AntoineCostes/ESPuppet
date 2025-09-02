# ESPuppet

ESP32 firmware for animatronics and connected stage props

GETTING STARTED
remove ESP32Async and RPAsyncTCP from ESPAsyncWebServer depedencies
which ESP32 platform to use ?

ROADMAP
OK /Dobbby filter
OK advertise endPacket
OK odrive module + optional server (or debug wroom server)
- safe gravity: efuse, bad UART
- ledModule advertise/notify -> wifi debug
- setPattern with CRGB color
- clean Modules: timers, fullMatch, check range
- birandom
- optionnaly clear before applying effect ?
- use targetModule & targetComponent
    leds/strip/set
    motors/servo/set
    gpio/output/set
    ou bien
    leds/strip/0/set 
    leds/strip/neon/set 
    ?
    mmh non c'est le premier argument qui est un int ou un String
    comme ça c'est toujours le module qui filtre les arguments

- no config => load default which starts AP
- button events in main.cpp ? or behavior inside Module ?
- webserver console
- webserver wifiscan
- targetIP in webserver => singleton master
- ServoPWMShield separate class + servo start
- pin management -> advertise config errors
- Parameters -> automated loadConfig + no handleOSCCommand: faut clarifier comment faire le callback


TO DIG
OK wifi set power
- differences Preferences / LittleFS: memmory size ?
- https://github.com/ayushsharma82/WebSerial
- handle if mDNS failed ?
- not sure if wifi host name is usefull for anything

IDEAS
- counter class
- allowed pins for each module and boards ?
- color templates for wifidebug (parameters)
- lastUpdateMs in component ?
- inputComponent génère des évènement, outputComponent est contrôlable par OSC ?
- FileManager Component ?
- print not found host/url on webserver
- variadic sendOSC + include OSC lib
- clarify serialDebug implementation

COSMETICS
- use props with cast
- FileManager singleton 
- char \* name
- variadic reservePins
- color instead of r, g, b in LedModule
