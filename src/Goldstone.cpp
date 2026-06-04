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
  typedef enum arloError {BUS_OUTPUT_BLOCKED, BUS_INPUT_BLOCKED, BUS_INPUT_OUTPUT_SET} error_t;
  typedef enum busState {NO_BUS = -1, BUS_INPUT = 0, BUS_OUTPUT = 1, BUS_ERROR = -2} bus_t;

  void setLsb(bool state);
  void setMsb(bool state);
  void flipLsb();
  void flipMsb();
  void shiftIn(byte bits);
  void clearRegister(bool latch = true);
  bus_t getBusRequirement(periph_t peripheral);
  bus_t getBusRequirements(byte peripheralBits);
  
  void addPeripherals(byte bits);
  void setPeripherals(byte bits);
  void unsetPeripherals(byte bits);
  void setError(arloError errorCode);
  void runBasicDiagnostic(periph_t peripheral);

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

    return;
    //temporary startup
    delay(3500);
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
    unsetPeripherals(BODY | LITE);
    delay(500);
    for (int i = 0; i < 100; i++) {
      shoulderServo.write((float)i / 100.0f * 170);
      if (i == 30 || i == 80) addPeripherals(BUZZ);
      if (i == 40 || i == 90) unsetPeripherals(BUZZ);

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
      }else {
        busServo.write((float)(i - 150) / 50.0f * 20 + 105);
        shoulderServo.write((float)(i - 150) / 50.0f * -10 + 180);
      }
      delay(5);
    }
  }

  void orbit() {
    
  }

  //============================== PERSONALITY & MODES ==============================

  //============================== MOTOR SKILLS ==============================

  

  //============================== UTILITIES & ACTIONS ==============================

  void setPeripherals(byte bits) { //AHHHHHHH DO BUS STATE CHANGES FOR SPECIFIC PERIPHERALS
    //validate & set bus
    bus_t desiredBus = getBusRequirements(bits);
    switch (desiredBus) {
      case -2:break; //ahhhhh make a git repo
    }
        
    
    
      shiftIn(bits);
      //set latch
      flipMsb();
      flipLsb();
      latchBits = registerBits;
  }  
  void addPeripherals(byte bits) {setPeripherals(bits | latchBits);}
  void unsetPeripherals(byte bits) {setPeripherals(~bits & latchBits);}


  void setError(arloError errorCode) {

  }

  void runBasicDiagnostic(periph_t peripheral) {
    
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
    shiftIn(0);
    if (latch) {
      flipMsb();
      flipLsb();
      latchBits = registerBits;
    }
  }

}
