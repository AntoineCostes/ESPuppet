#include "LedModule.h"

LedModule::LedModule() : Module("leds")
{
}

void LedModule::init()
{
}

void LedModule::update()
{
    for (auto const &strip : strips) strip->update();
}

void LedModule::loadConfig(JsonObject const& config)
{
    for (JsonPair kv : config) {
        registerLedStrip(config[kv.key()]);
    }
}

void LedModule::registerLedStrip(JsonObject const& config)
{
    // bool serialDebug = config["serialDebug"] | false;
    int pin = config["pin"] | -1;
    int numPixels = config["numPixels"] | -1;
    float brightness = config["brightness"] | 0.5f;
    bool wifiDebug = config["wifiDebug"] | false;

    if (pin > 0 && numPixels > 0) 
        registerLedStrip(pin, numPixels, brightness);
    else err("cannot register ledstrip, pin & numPixels should be positive !");
}

void LedModule::registerLedStrip(int pin, int numPixels, float brightness, neoPixelType type)
{
    if (Module::reservePin(pin))
        strips.emplace_back(new LedStrip(pin, numPixels, brightness, type));
    else err("cannot register ledstrip, pin"+String(pin)+" is reserved");
}

void LedModule::clear(uint8_t index)
{
    if (index < 0 || index >= strips.size()) return;
    strips[index]->clear();
}

void LedModule::clearAll()
{
    for (auto const &strip : strips) strip->clear();
}

void LedModule::setSolid(uint8_t index, uint8_t r, uint8_t g, uint8_t b)
{
    if (index < 0 || index >= strips.size()) return;
    strips[index]->setSolid(r, g, b);
}

void LedModule::setWave(uint8_t index, uint8_t r, uint8_t g, uint8_t b, float frequency)
{
    if (index < 0 || index >= strips.size()) return;
    strips[index]->setWave(r, g, b, frequency);

}

void LedModule::setBlink(uint8_t index, uint8_t r, uint8_t g, uint8_t b, float frequency)
{
    if (index < 0 || index >= strips.size()) return;
    strips[index]->setBlink(r, g, b, frequency);

}
