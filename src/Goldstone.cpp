#include "Novae.h"

#define TIME2METER 0.00017f
#define METER2TIME 5882.352941f

#define BUS_OUTPUTS BODY | ARM_L
#define BUS_INPUTS KILL | ECHO

using namespace cmb;

namespace Goldstone {
  uint8_t demulLsbPin;
  uint8_t demulMsbPin;
  uint8_t shoulderPin;
  uint8_t headPin;
  uint8_t busPin;

  Servo headServo;
  Servo shoulderServo;
  Servo busServo;

  bool demulLsb;
  bool demulMsb;
  byte registerBits;
  byte latchBits;

  bool currentBusState = INPUT;

  typedef enum arloPeripheral {ARM_L = 1, ECHO = 2, KILL = 4, BODY = 8, LITE = 16, LASR = 32, BUZZ = 64, TRIG = 128} periph_t;
  typedef enum arloDiagnostic {SERVO_ARM_L = 1, TRIG_ECHO = 2, CAPACITIVE_SENSOR = 4, SERVO_BODY = 8, LED = 16, LASER = 32, BUZZER = 64, SERVO_ARM_U, SERVO_HEAD, SERVO_ALL, CALIBRATE_ARM_L, CALIBRATE_ARM_U, CALIBRATE_BODY, CALIBRATE_HEAD, CALIBRATE_ALL} diagnostic_t;
  typedef enum arloError {NEGATIVE, POSITIVE, BUS_OUTPUT_BLOCKED, BUS_INPUT_BLOCKED, BUS_INPUT_OUTPUT_SET} error_t;
  typedef enum killState {NOT_SENSING, NO_TOUCH, TOUCH} kill_t;
  typedef enum busState {NO_BUS = -1, BUS_INPUT = 0, BUS_OUTPUT = 1, BUS_ERROR = -2} bus_t;

  int killNoTouchBaseline;
  int killTouchBaseline;
  int killHighSubtractor = 1000; //randomly analogReading kill will output a regularvalue plus 1000 exactly, this accounts for that when it occurs

  //registers & bits
  void setLsb(bool state);
  void setMsb(bool state);
  void flipLsb();
  void flipMsb();
  void shiftIn(byte bits);
  void clearRegister(bool latch = true);

  //utilities & actions
  bool getPeripheral(periph_t peripheral);
  void setPeripherals(byte bits);
  void addPeripherals(byte bits);
  void removePeripherals(byte bits);
  void setError(arloError errorCode);
  void runDiagnostic(diagnostic_t diagnosis);
  kill_t getKillState();
  kill_t getKillStateRaw(bool trySetKillPeripherial = false);
  void calibrateKill();
  
  bus_t getCurrentBusState();
  bus_t getBusRequirement(periph_t peripheral);
  bus_t getBusRequirements(byte peripheralBits);

  //motor skills
  

  //personality management


  //==========================================================================================================

  void proto(uint8_t pinTX_plus, uint8_t pinTX_minus, uint8_t pwmRX_plus, uint8_t pwmRX_minus, uint8_t pwmSBU) {
    demulMsbPin = pinTX_minus;
    demulLsbPin = pinTX_plus;
    shoulderPin = pwmRX_plus;
    headPin = pwmRX_minus;
    busPin = pwmSBU;

    pinMode(demulLsbPin, OUTPUT);
    pinMode(demulMsbPin, OUTPUT);
    pinMode(shoulderPin, OUTPUT);
    pinMode(headPin, OUTPUT);
    pinMode(busPin, INPUT);
    
    setMsb(true);
    setLsb(true);

    clearRegister();
    
    headServo.attach(headPin);
    shoulderServo.attach(shoulderPin);
    busServo.attach(busPin);

    clearRegister(false);
    setPeripherals((byte)ARM_L);
    busServo.write(95);
    pinMode(busPin, OUTPUT);
    delay(500);

    pinMode(busPin, INPUT);
    headServo.write(90);
    shoulderServo.write(0);

    //working startup sequence
    calibrateKill();
    runDiagnostic(CAPACITIVE_SENSOR);
    return;
    //temporary startup
    delay(100);
    setPeripherals(BODY);
    busServo.write(90);
    delay(3500);
    clearRegister();
    for (int i = 0; i < 100; i++) {
      byte currentPeriphs = 0;

      if (i % 7 == 0) currentPeriphs |= LITE;
      if (i % 18 == 0) {
        currentPeriphs |= BUZZ;
      }

      headServo.write((float)i / 100.0f * 90 + 90);
      setPeripherals(currentPeriphs);
      if(currentPeriphs & BUZZ) delay(50);
      else delay(10);
    }
    setPeripherals(0);
    delay(1500);

  
    setPeripherals(BODY | LITE);
    pinMode(busPin, OUTPUT);
    busServo.write(180);
    delay(30);
    headServo.write(90);
    delay(250);
    removePeripherals(BODY | LITE);
    delay(500);
    for (int i = 0; i < 100; i++) {
      shoulderServo.write((float)i / 100.0f * 170);
      if (i == 30 || i == 80) addPeripherals(BUZZ);
      if (i == 40 || i == 90) removePeripherals(BUZZ);

      if (i == 50) addPeripherals(ARM_L);

      busServo.write((float)(i - 50) / 50.0f * 10 + 95);
      delay(4);
    }
    
    for (int i = 0; i < 200; i++) {
      
      if (i < 50) {
        busServo.write((float)(i) / 50.0f * -20 + 105);
        shoulderServo.write((float)(i) / 50.0f * 10 + 170);
    }
      else if 
        ( i < 100){ busServo.write((float)(i - 50) / 50.0f * 20 + 105);
        shoulderServo.write((float)(i - 50) / 50.0f * -10 + 180);
      } else if (i < 150) {
        busServo.write((float)(i - 100) / 50.0f * -20 + 105);
        shoulderServo.write((float)(i - 100) / 50.0f * 10 + 170);
      } else {
        busServo.write((float)(i - 150) / 50.0f * 20 + 105);
        shoulderServo.write((float)(i - 150) / 50.0f * -10 + 180);
      }
      delay(5);
    }
  }

  void orbit() {
    runDiagnostic(CAPACITIVE_SENSOR);
  }

  //============================== PERSONALITY MANAGEMENT ==============================

  //============================== MOTOR SKILLS ==============================
  // when switching between arm and body movement, there needs to be a small delay or else the servo set first gets confused

  //============================== UTILITIES & ACTIONS ==============================
  bool getPeripheral(periph_t peripheral) {
    return latchBits & peripheral;
  }

  void setPeripherals(byte bits) {
    //validate & set bus
    bool setOutputAfterLatch = false;
    bus_t desiredBus = getBusRequirements(bits);
    if (bits == BODY) Serial.println(desiredBus);
    switch (desiredBus) {
      case BUS_ERROR:
        setError(BUS_INPUT_OUTPUT_SET);
        return;

      case BUS_INPUT:
        pinMode(busPin, INPUT);
        break;

      case BUS_OUTPUT:
          pinMode(busPin, INPUT);
          setOutputAfterLatch = true;
        break;

      case NO_BUS:
        pinMode(busPin, INPUT);
        break;
    }

    //set peripherals if bus was valid
    shiftIn(bits);
    //set latch
    flipMsb();
    flipLsb();
    latchBits = registerBits;
    if (setOutputAfterLatch) {
      pinMode(busPin, OUTPUT);
    }
  }  

  void addPeripherals(byte bits) {setPeripherals(bits | latchBits);}
  void removePeripherals(byte bits) {setPeripherals(~bits & latchBits);}


  void setError(arloError errorCode) {

  }

  void runDiagnostic(diagnostic_t diagnosis) {
    setPeripherals(0);

    switch (diagnosis) {
      case TRIG_ECHO:
        break;

      case CAPACITIVE_SENSOR: {
        setPeripherals(KILL | LITE | LASER);
        const unsigned long timeoutSeconds = 8;
        unsigned long timeoutTimestamp = millis() + 1000 * timeoutSeconds;
        bool diagnosticResult = false;

        while (millis() < timeoutTimestamp) { //wait for sensor touch
          if (getKillState() == TOUCH) {
            addPeripherals(BUZZ);
            removePeripherals(LITE | LASER);
            timeoutTimestamp = millis() + 1000 * timeoutSeconds;

            while (millis() < timeoutTimestamp) { //wait for sensor touch to stop
              if (getKillState() == NO_TOUCH) {
                diagnosticResult = true;
                timeoutTimestamp = 0;
              }
            }
          }
        } 
        
        if (diagnosticResult == false) {
          setError(NEGATIVE);
        } else {
          setPeripherals(LITE | LASER);
          delay(50);
          setPeripherals(0);
          delay(50);
          setError(POSITIVE);
        }
        setPeripherals(0);
        break;
      }

      case LED:
                
        break;
      
      case LASER:
        break;
      
      case BUZZER:
        break;

      case ARM_L:
        
      case BODY:
      
        break;
    }
  }

  bus_t getBusRequirement(arloPeripheral peripheral) { //-1: bus not used, OUTPUT, INPUT
    bus_t busState = NO_BUS;
    if (peripheral & BUS_OUTPUTS == true)
      busState = BUS_OUTPUT;
    else if (peripheral & BUS_INPUTS == true) 
      busState = BUS_INPUT;
    
    return busState;
  }

  bus_t getBusRequirements(byte peripheralBits) { //-1: bus not used, OUTPUT, INPUT, -2 error
    bool isOutput = false;
    bool isInput = false;
    for (int i = 0; i < 8; i++) {
      periph_t currentPeriph = (periph_t)(peripheralBits & (1 << i));
      if (getBusRequirement(currentPeriph) == BUS_OUTPUT) isOutput = true; 
      if (getBusRequirement(currentPeriph) == BUS_INPUT) isInput = true; 
    }
    
    if (isOutput && isInput) {
      return BUS_ERROR;
    } else if (isOutput) {
      return BUS_OUTPUT;
    } else if (isInput) {
      return BUS_INPUT;
    } else return NO_BUS;
  }

  bus_t getCurrentBusState() {
    bus_t busState = getBusRequirements(latchBits);
    if (busState == BUS_ERROR) {
      setError(BUS_INPUT_OUTPUT_SET);
    }
    return busState;
  }

  kill_t getKillState() {
    int doubleTries = 1; //averages data from (2 * doubleTries + 1) sensor reads
    int total = 0;

    if (getKillStateRaw(true) == TOUCH) total++;
    for (int i = 0; i < doubleTries; i++) {
      if (getKillStateRaw() == TOUCH) total++;
    }
    return total >= doubleTries + 1 ? TOUCH : NO_TOUCH;
  }

  kill_t getKillStateRaw(bool trySetKillPeripherial) { 
    if (!getPeripheral(KILL))
      return NOT_SENSING;

    if (trySetKillPeripherial) {
      if (getBusRequirements(latchBits | KILL) == BUS_ERROR)
        return NOT_SENSING;
      else
        addPeripherals(KILL);
    }

    
    int sensorValue = analogRead(busPin);
    if (sensorValue > killHighSubtractor) sensorValue -= killHighSubtractor;
    return sensorValue > killTouchBaseline ? TOUCH : NO_TOUCH;
  }

  void calibrateKill() {
    setPeripherals(KILL);
    int sampleCount = 10;
    int samples[sampleCount];
    for (int i = 0; i < sampleCount; i++) {
      delay(5);
      samples[i] = analogRead(busPin);
    }

    int mode = getMode(samples, sampleCount);
    killNoTouchBaseline = mode;
    killTouchBaseline = mode + 9;
  }

  //============================== REGISTERS & BITS ==============================

  void setLsb(bool state) {demulLsb = state; digitalWrite(demulLsbPin, !state);}
  void setMsb(bool state) {demulMsb = state; digitalWrite(demulMsbPin, !state);}
  void flipLsb() {demulLsb = !demulLsb; digitalWrite(demulLsbPin, !demulLsb);}
  void flipMsb() {demulMsb = !demulMsb; digitalWrite(demulMsbPin, !demulMsb);}

  void shiftIn(byte bits) {
    //get the demul bits to 10
    if (demulMsb == false && demulLsb == true) //01
      setLsb(false);
    if (demulLsb == demulMsb) {
      if (demulMsb == false) //00
        setMsb(true);
      else //11
        setLsb(false);
    }

    //shift bits in
    for (int i = 7; i >= 0; i--) {
      bool currentBit = (bits >> i) & 1;
      if (currentBit == true) {
        flipLsb(); flipLsb();
      }
      else {
        flipMsb(); flipMsb();
      }
    }
    registerBits = bits;
  }

  void clearRegister(bool latch) {
    pinMode(busPin, INPUT);
    shiftIn(0);
    if (latch) {
      flipMsb();
      flipLsb();
      latchBits = registerBits;
    }
  }

}
