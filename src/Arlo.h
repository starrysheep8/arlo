#pragma once
#include <Arduino.h>
#include <Servo.h>
#include "Motion.h"

#define TIME2METER 0.00017f
#define METER2TIME 5882.352941f

#define BUS_OUTPUTS BODY | ARM_L
#define BUS_INPUTS KILL | ECHO

namespace Arlo {
    void proto(uint8_t pinTX_plus, uint8_t pinTX_minus, uint8_t pwmRX_plus, uint8_t pwmRX_minus, uint8_t pwmSBU);
    void orbit();

    //============================== QUIRKS ==============================

    //============================== MOTORS ==============================
    extern Servo headServo;
    extern Servo armUpperServo;
    extern Servo busServo;

    extern Motion headMotion;
    extern Motion armUpperMotion;
    extern Motion armLowerMotion;
    extern Motion bodyMotion;

    void motionHandler();

    namespace Angles {
        const int alStraight = 95;
    }

    //============================== UTILS ==============================
    //Diagnostics
    typedef enum arloError {NEGATIVE, POSITIVE, BUS_OUTPUT_BLOCKED, BUS_INPUT_BLOCKED, BUS_INPUT_OUTPUT_SET, BAD_PERIPHERAL} error_t;
    typedef enum arloDiagnostic {SERVO_ARM_L = 1, TRIG_ECHO = 2, CAPACITIVE_SENSOR = 4, SERVO_BODY = 8, LED = 16, LASER = 32, BUZZER = 64, SERVO_ARM_U, SERVO_HEAD, SERVO_ALL, CALIBRATE_ARM_L, CALIBRATE_ARM_U, CALIBRATE_BODY, CALIBRATE_HEAD, CALIBRATE_ALL} diagnostic_t;

    void setError(arloError errorCode);
    void runDiagnostic(diagnostic_t diagnosis);
    //

    extern int killNoTouchBaseline;
    extern int killTouchBaseline;
    extern int killHighSubtractor;

    typedef enum arloPeripheral {ARM_L = 1, ECHO = 2, KILL = 4, BODY = 8, LITE = 16, LASR = 32, BUZZ = 64, TRIG = 128} periph_t; //AHHHHH MOVE THIS TO ARLO.H
    typedef enum busState {NO_BUS = -1, BUS_INPUT = 0, BUS_OUTPUT = 1, BUS_ERROR = -2} bus_t;
    typedef enum killState {NOT_SENSING, NO_TOUCH, TOUCH} kill_t;

    bool getPeripheral(periph_t peripheral);
    void setPeripherals(byte bits);
    void addPeripherals(byte bits);
    void removePeripherals(byte bits);
    kill_t getKillState();
    kill_t getKillStateRaw(bool trySetKillPeripherial = false);
    void calibrateKill();
  
    bus_t getCurrentBusState();
    bus_t getBusRequirement(periph_t peripheral);
    bus_t getBusRequirements(byte peripheralBits);

    bool arrayContains(int element, int array[], int size);
    int getMode(int intArray[], int size);

    //============================== DRIVERS ==============================
    extern uint8_t demulLsbPin;
    extern uint8_t demulMsbPin;
    extern uint8_t armUpperPin;
    extern uint8_t headPin;
    extern uint8_t busPin;

    extern bool demulLsb;
    extern bool demulMsb;
    extern byte registerBits;
    extern byte latchBits;

    void setLsb(bool state);
    void setMsb(bool state);
    void flipLsb();
    void flipMsb();

    void shiftIn(byte bits);
    void clearRegister(bool latch = true);

}