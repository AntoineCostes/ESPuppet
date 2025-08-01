#include "ODriveMotor.h"

template<class T> inline Print& operator <<(Print &obj,     T arg) { obj.print(arg);    return obj; }
template<>        inline Print& operator <<(Print &obj, float arg) { obj.print(arg, 4); return obj; }


ODriveMotor::ODriveMotor(String name, Stream& uart, int motorIndex):Component(name), uart(uart), motorIndex(motorIndex)
{
    controlMode = ODriveControlMode::CONTROL_MODE_VOLTAGE_CONTROL;
    inputMode = ODriveInputMode::INPUT_MODE_INACTIVE;
}

void ODriveMotor::init()
{
    setState(ODriveAxisState::AXIS_STATE_CLOSED_LOOP_CONTROL);
    delay(10);
    ODriveAxisState state = (ODriveAxisState)readString().toInt();
    while (state != ODriveAxisState::AXIS_STATE_CLOSED_LOOP_CONTROL ||state != ODriveAxisState::AXIS_STATE_IDLE)
    {
        setState(ODriveAxisState::AXIS_STATE_CLOSED_LOOP_CONTROL);
        delay(100);
        Serial.println("try hard");
        state = (ODriveAxisState)readString().toInt();
    }
}

void ODriveMotor::clearErrors()
{
    Serial.println("CLEAR ERRORS");
    uart << F("sc\n");
}

void ODriveMotor::setPosition(float position, float velocity_feedforward, float torque_feedforward)
{
    if (hasError)
    {
        init();
        delay(10);
        hasError = false;
    }
    if (controlMode != ODriveControlMode::CONTROL_MODE_POSITION_CONTROL)
    {
        Serial.println("Set control mode: POS");
        setParameter("control_mode", ODriveControlMode::CONTROL_MODE_POSITION_CONTROL);
        setParameter("input_mode", ODriveInputMode::INPUT_MODE_PASSTHROUGH); 
        setParameter("vel_limit", 10.0f);
        
        controlMode = ODriveControlMode::CONTROL_MODE_POSITION_CONTROL;
    }
    uart << F("p ") << motorIndex  << F(" ") << position << F(" ") << velocity_feedforward << F(" ") << torque_feedforward << F("\n");
}


void ODriveMotor::setState(ODriveAxisState newState)
{
    uart << "w axis" << motorIndex << ".requested_state " << newState << '\n';
}
void ODriveMotor::setVelocity(float velocity, float torque_feedforward)
{
    if (hasError)
    {
        init();
        delay(10);
        hasError = false;
    }
    if (controlMode != ODriveControlMode::CONTROL_MODE_VELOCITY_CONTROL)
    {
        Serial.println("Set control mode: VEL");
        setParameter("control_mode", ODriveControlMode::CONTROL_MODE_VELOCITY_CONTROL);
        setParameter("input_mode", ODriveInputMode::INPUT_MODE_PASSTHROUGH); 
        setParameter("vel_limit", 10.0f);
        controlMode = ODriveControlMode::CONTROL_MODE_VELOCITY_CONTROL;
    }
    uart << F("v ") << motorIndex  << F(" ") << velocity << F(" ") << torque_feedforward << F("\n");
}

void ODriveMotor::setTorque(float torque)
{
    if (hasError)
    {
        init();
        delay(10);
        hasError = false;
    }
    if (controlMode != ODriveControlMode::CONTROL_MODE_TORQUE_CONTROL)
    {
        Serial.println("Set control mode: TORQUE");
        setParameter("control_mode", ODriveControlMode::CONTROL_MODE_TORQUE_CONTROL);
        setParameter("input_mode", ODriveInputMode::INPUT_MODE_PASSTHROUGH); 
        float torque_cst = 8.23 / 150;
        setParameter("torque_constant", torque_cst);
        setParameter("current_lim", 5.0f);
        controlMode = ODriveControlMode::CONTROL_MODE_TORQUE_CONTROL;
    }
    uart << F("c ") << motorIndex << F(" ") << torque << F("\n");
}

void ODriveMotor::setParameter(const String& parameter, const String& value)
{
    uart << "w " << "axis"+String(motorIndex)+".controller.config."+parameter << " " << value << "\n";
}

void ODriveMotor::setParameter(const String& parameter, int value)
{
    setParameter(parameter, String(value));
}

void ODriveMotor::setParameter(const String& parameter, float value)
{
    setParameter(parameter, String(value));
}

float ODriveMotor::getPos()
{
    return position;
}

float ODriveMotor::getVel()
{
    return velocity;
}

void ODriveMotor::update()
{
    // Serial.println("update");

    // Flush RX
    if (uart.available()) Serial.println("FLUSH uart");
    while (uart.available()) Serial.print( (char) uart.read() );
    
	uart << "r axis" << motorIndex << ".pos_estimate\n";
    position = readString().toFloat();

	uart << "r axis" << motorIndex << ".vel_estimate\n";
    velocity = readString().toFloat();

	uart << "r axis" << motorIndex << ".current_state\n";
    ODriveAxisState newState = (ODriveAxisState)readString().toInt();
    if (newState != state)
    {
        if (newState >= 17) Serial.println("unkown state");
        else Serial.println("NEW STATE: "+stateNames[newState]);
        state = newState;
    }
    
	uart << "r axis" << motorIndex << ".active_errors\n";
    ODriveError newError = (ODriveError)readString().toInt();
    if (newError != error)
    {
        if (newError >= 24) Serial.println("unkown error");
        else Serial.println("NEW ERROR: "+errorNames[newError]);
        error = newError;
        if (error > 0) hasError = true;
    }
    
	uart << "r axis" << motorIndex << ".disarm_reason\n";
    ODriveError newReason = (ODriveError)readString().toInt();
    if (newReason != reason)
    {
        if (newReason >= 24) Serial.println("unkown reason");
        Serial.println("NEW REASON: "+errorNames[newReason]);
        reason = newReason;
    }
}

String ODriveMotor::readString()
{
    String str = "";
    long timeout = 200;
    unsigned long startReadingTime = millis();
    long readTime = 0;
    for (;;)
{
        while (!uart.available())
    {
        readTime = millis() - startReadingTime;
            if (readTime >= timeout)
            {
                Serial.println("ERROR timeout expired");
                return str;
            }
        }
        char c = uart.read();
        if (c == '\n')
            break;
        str += c;
    }
    Serial.println("got : "+str);
    if (readTime>5) Serial.println("read time = "+String(readTime));
    return str;
}
