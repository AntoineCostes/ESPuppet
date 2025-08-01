#pragma once
#include "ODriveDefinitions.h"
#include "common/Component.h"

class ODriveMotor : public Component 
{
public:
    ODriveMotor(String name, Stream& uart, int motorIndex);
    void clearErrors();
    void init();

    void setPosition(float position, float velocity_feedforward, float torque_feedforward);
    void setVelocity(float velocity, float torque_feedforward);
    void setTorque(float torque);

    float getPos();
    float getVel();
    void update();

protected:
    Stream& uart;
    String readString();
    bool hasError;

    int motorIndex;
    float position;
    float velocity;
    ODriveAxisState state;
    ODriveError error;
    ODriveError reason;
    
    
    void setState(ODriveAxisState newState);
    void setParameter(const String& parameter, const String& value);
    void setParameter(const String& parameter, int value);
    void setParameter(const String& parameter, float value);

    ODriveControlMode controlMode;
    ODriveInputMode inputMode;

    
    String stateNames[17] = 
    {
    "UNDEFINED",
    "IDLE",
    "STARTUP_SEQUENCE",
    "FULL_CALIBRATION_SEQUENCE",
    "MOTOR_CALIBRATION",
    "ENCODER_INDEX_SEARCH",
    "ENCODER_OFFSET_CALIBRATION",
    "CLOSED_LOOP_CONTROL",
    "LOCKIN_SPIN",
    "ENCODER_DIR_FIND",
    "HOMING",
    "ENCODER_HALL_POLARITY_CALIBRATION",
    "ENCODER_HALL_PHASE_CALIBRATION",
    "ANTICOGGING_CALIBRATION",
    "HARMONIC_CALIBRATION",
    "HARMONIC_CALIBRATION_COMMUTATION"
    };
    
String errorNames[24] = 
{
"NONE",
"INITIALIZING",
"SYSTEM_LEVEL",
"TIMING_ERROR",
"MISSING_ESTIMATE",
"BAD_CONFIG",
"DRV_FAULT",
"MISSING_INPUT",
"DC_BUS_OVER_VOLTAGE",
"DC_BUS_UNDER_VOLTAGE",
"DC_BUS_OVER_CURRENT",
"DC_BUS_OVER_REGEN_CURRENT",
"CURRENT_LIMIT_VIOLATION",
"MOTOR_OVER_TEMP",
"INVERTER_OVER_TEMP",
"VELOCITY_LIMIT_VIOLATION",
"POSITION_LIMIT_VIOLATION",
"WATCHDOG_TIMER_EXPIRED",
"WATCHDOG_TIMER_EXPIRED",
"ESTOP_REQUESTED",
"SPINOUT_DETECTED",
"BRAKE_RESISTOR_DISARMED",
"THERMISTOR_DISCONNECTED",
"CALIBRATION_ERROR"
};

};