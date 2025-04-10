#include "LedModule.h"

LedModule::LedModule() : Module("leds")
{
}

void LedModule::init()
{
}

void LedModule::update()
{
    for (auto const &strip : strips)
        strip->update();
}

void LedModule::loadConfig(JsonObject const &config)
{
    serialDebug = config["serialDebug"] | false;
    dbg("load config");

    for (JsonPair kv : config)
    {
        if (kv.value().is<JsonObject>())
            registerLedStrip(config[kv.key()]);
    }
}

void LedModule::registerLedStrip(JsonObject const &config)
{
    int pin = config["pin"] | -1;
    int numPixels = config["numPixels"] | -1;
    float brightness = config["brightness"] | 0.5f;
    bool wifiDebug = config["wifiDebug"] | false;

    if (pin > 0 && numPixels > 0)
        registerLedStrip(pin, numPixels, brightness);
    else
        err("cannot register ledstrip, pin & numPixels should be positive !");
}

void LedModule::registerLedStrip(int pin, int numPixels, float brightness, neoPixelType type)
{
    if (Module::reservePin(pin))
        strips.emplace_back(new LedStrip(pin, numPixels, brightness, type));
    else
        err("cannot register ledstrip, pin" + String(pin) + " is reserved");
}

void LedModule::clear(uint8_t index)
{
    if (index < 0 || index >= strips.size())
        return;
    strips[index]->clear();
}

void LedModule::clearAll()
{
    for (auto const &strip : strips)
        strip->clear();
}

void LedModule::setSolidAll(uint8_t r, uint8_t g, uint8_t b)
{
    for (int i = 0; i < strips.size(); i++)
        setSolid(i, r, g, b);
}

void LedModule::setSolid(uint8_t index, uint8_t r, uint8_t g, uint8_t b)
{
    if (index < 0 || index >= strips.size())
        return;
    strips[index]->setSolid(r, g, b);
}

void LedModule::setWaveAll(uint8_t r, uint8_t g, uint8_t b, float frequency)
{
    for (int i = 0; i < strips.size(); i++)
        setWave(i, r, g, b, frequency);
}

void LedModule::setWave(uint8_t index, uint8_t r, uint8_t g, uint8_t b, float frequency)
{
    if (index < 0 || index >= strips.size())
        return;
    strips[index]->setWave(r, g, b, frequency);
}

void LedModule::setBlinkAll(uint8_t r, uint8_t g, uint8_t b, float frequency)
{
    for (int i = 0; i < strips.size(); i++)
        setBlink(i, r, g, b, frequency);
}

void LedModule::setBlink(uint8_t index, uint8_t r, uint8_t g, uint8_t b, float frequency)
{
    if (index < 0 || index >= strips.size())
        return;
    strips[index]->setBlink(r, g, b, frequency);
}

void LedModule::handleOSCCommand(OSCMessage *command)
{
    if (command->match("/led/color"))
    {
        if (command->size() == 3)
        {
            if (command->isInt(0) && command->isInt(1) && command->isInt(2))
            {
                uint8_t r = command->getInt(0);
                uint8_t g = command->getInt(1);
                uint8_t b = command->getInt(2);
                setSolidAll(r, g, b);
            }
            else if (command->isFloat(0) && command->isFloat(1) && command->isFloat(2))
            {
                uint8_t r = command->getFloat(0)*255;
                uint8_t g = command->getFloat(1)*255;
                uint8_t b = command->getFloat(2)*255;
                setSolidAll(r, g, b);
            }
        }
        else if (command->size() == 4)
        {
            if (command->isInt(0) && command->isInt(1) && command->isInt(2) && command->isInt(3))
            {
                uint8_t index = command->getInt(0);
                uint8_t r = command->getInt(1);
                uint8_t g = command->getInt(2);
                uint8_t b = command->getInt(3);
                setSolid(index, r, g, b);
            }
            else if (command->isInt(0) && command->isFloat(1) && command->isFloat(2) && command->isFloat(3))
            {
                uint8_t index = command->getInt(0);
                uint8_t r = command->getFloat(1)*255;
                uint8_t g = command->getFloat(2)*255;
                uint8_t b = command->getFloat(3)*255;
                setSolid(index, r, g, b);
            }
        }
    } 
}