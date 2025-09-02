#include "ODriveModule.h"

ODriveModule::ODriveModule() : Module("odrive")
{
}

void ODriveModule::init()
{
}

void ODriveModule::update()
{
    for (auto const &motor : motors)
        motor->update();
}

void ODriveModule::loadConfig(JsonObject const &config)
{
    if (config) Serial.println("");
    serialDebug = config["serialDebug"] | false;
    
    int uart = config["uart"]| -1;
    int rx = config["rx"]| -1;
    int tx = config["tx"]| -1; 
    
    if (uart >= 0)
        registerOdrive(uart, rx, tx);
}

void ODriveModule::registerOdrive(int uartIndex, int rx, int tx)
{
    HardwareSerial uart(uartIndex);
    if (rx >= 0 && tx >= 0) uart.begin(BAUDRATE, SERIAL_8N1, rx, tx);
    else uart.begin(BAUDRATE, SERIAL_8N1);
    dbg(String(uartIndex));
    dbg(String(rx));
    dbg(String(tx));
    while (!uart) ; 
    dbg("UART Ready...");
    motors.emplace_back(new ODriveMotor("odrive_"+String(uartIndex), uart, motors.size()));
    motors[motors.size()-1]->init();
}

void ODriveModule::setPosition(int index, float value)
{
    if (index < 0 || index >= motors.size())
    {
        err("invalid output index: " + String(index) + " while it should be between 0 and " + String(motors.size() - 1));
        return;
    }
    motors[index]->setPosition(value, 0.0, 0.0);
}

void ODriveModule::setVelocity(int index, float value)
{
    if (index < 0 || index >= motors.size())
    {
        err("invalid output index: " + String(index) + " while it should be between 0 and " + String(motors.size() - 1));
        return;
    }
    motors[index]->setVelocity(value, 0.0);
}

void ODriveModule::setTorque(int index, float value)
{
    if (index < 0 || index >= motors.size())
    {
        err("invalid output index: " + String(index) + " while it should be between 0 and " + String(motors.size() - 1));
        return;
    }
    motors[index]->setTorque(value);
}

void ODriveModule::handleOSCCommand(OSCMessage *command)
{
    if (command->fullMatch("/odrive/pos"))
    {
        if (command->isFloat(0))
            setPosition(0, command->getFloat(0));

        if (command->isInt(0) && command->isFloat(1))
            setPosition(command->getInt(0), command->getFloat(1));
    }

    if (command->fullMatch("/odrive/vel"))
    {
        if (command->isFloat(0))
            setVelocity(0, command->getFloat(0));

        if (command->isInt(0) && command->isFloat(1))
            setVelocity(command->getInt(0), command->getFloat(1));
    }

    if (command->fullMatch("/odrive/torque"))
    {
        if (command->isFloat(0))
            setTorque(0, command->getFloat(0));

        if (command->isInt(0) && command->isFloat(1))
            setTorque(command->getInt(0), command->getFloat(1));
    }
}