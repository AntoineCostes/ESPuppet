#include "LedModule.h"

LedModule::LedModule() : Module("ledstrip")
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
    bool grb = config["grb"] | true;
    
    if (pin > 0 && numPixels > 0)
        registerLedStrip(pin, numPixels, brightness, grb?NEO_GRB:NEO_RGB + NEO_KHZ800);
    else
        err("cannot register ledstrip, pin ("+ String(pin)+") & numPixels ("+String(numPixels)+") should be positive !");
}

void LedModule::registerLedStrip(int pin, int numPixels, float brightness, neoPixelType type)
{
    if (Module::reservePin(pin))
    {
        dbg("Register strip with "+String(numPixels)+ " leds on pin #"+ String(pin));
        strips.emplace_back(new LedStrip(pin, numPixels, brightness, type));
    }
    else
        err("cannot register ledstrip, pin" + String(pin) + " is reserved");
}

void LedModule::clear(uint8_t index)
{
    if (index < 0 || index >= strips.size())
    {
        // TODO Module::checkIndex ?
        err("invalid ledstrip index: "+String(index)+ "while it should be between 0 and "+String(strips.size()));
        return;
    }
    strips[index]->clear();
}

void LedModule::clearAll()
{
    for (auto const &strip : strips)
        strip->clear();
}

void LedModule::setPattern(LedPattern pattern, uint8_t r, uint8_t g, uint8_t b, float parameter)
{
    for (int i = 0; i < strips.size(); i++)
        setPattern(i, pattern, r, g, b, parameter);
}

void LedModule::setPattern(uint8_t index,LedPattern pattern, uint8_t r, uint8_t g, uint8_t b, float parameter)
{
    if (index < 0 || index >= strips.size())
    {
        err("invalid ledstrip index: "+String(index)+ "while it should be between 0 and "+String(strips.size()));
        return;
    }
    dbg("set pattern "+String(pattern) +" for strip #"+String(index)+" with param = "+String(parameter));
    uint32_t color = ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
    strips[index]->setPattern(pattern, color, parameter);
}

void LedModule::handleOSCCommand(OSCMessage *command)
{
    if (command->match("/ledstrip/set"))
    {
        if (command->size() == 5 || command->size() == 6) // index, mode, r, g, b, (parameter)
        {
            if (command->isInt(0) && command->getInt(0) >= 0)
            {
                int index = command->getInt(0);
                if (command->isInt(1) && command->getInt(1) >= 0)
                {
                    LedPattern pattern = static_cast<LedPattern>(command->getInt(1));
                    float parameter = 1.0f;
                    if (command->size() == 6 && command->isFloat(5)) parameter = command->getFloat(5);
                    if (pattern == LedPattern::BLINK || pattern == LedPattern::OSCILLATOR) parameter /= 10.0f;

                    if (command->isInt(2) && command->isInt(3) && command->isInt(4))
                    {
                        uint8_t r = command->getInt(2);
                        uint8_t g = command->getInt(3);
                        uint8_t b = command->getInt(4);
                        setPattern(index, pattern, r, g, b, parameter);
                    }
                    else if (command->isFloat(2) && command->isFloat(3) && command->isFloat(4))
                    {
                        uint8_t r = command->getFloat(2)*255;
                        uint8_t g = command->getFloat(3)*255;
                        uint8_t b = command->getFloat(4)*255;
                        setPattern(index, pattern, r, g, b, parameter);
                    }
                    else if (command->isDouble(2)) dbg("DOUBLE");
                    else err("args 2, 3, 4 should be int or float");
                } else err("arg 1 should be positive int for mode");
            } else err("arg 0 should be positive int for index");
        }
        else
        {
            err("invalid number arguments for "+String(command->getAddress()));
        }
    } 
    else
    {
        err("unkown OSC command: "+String(command->getAddress()));
    }
}