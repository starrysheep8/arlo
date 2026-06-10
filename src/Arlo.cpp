#include "Arlo.h"

namespace Arlo {

  void proto(uint8_t pinTX_plus, uint8_t pinTX_minus, uint8_t pwmRX_plus, uint8_t pwmRX_minus, uint8_t pwmSBU) {
    demulMsbPin = pinTX_minus;
    demulLsbPin = pinTX_plus;
    armUpperPin = pwmRX_plus;
    headPin = pwmRX_minus;
    busPin = pwmSBU;

    pinMode(demulLsbPin, OUTPUT);
    pinMode(demulMsbPin, OUTPUT);
    pinMode(armUpperPin, OUTPUT);
    pinMode(headPin, OUTPUT);
    pinMode(busPin, INPUT);
    
    setMsb(true);
    setLsb(true);

    clearRegister();
    
    headServo.attach(headPin);
    armUpperServo.attach(armUpperPin);
    busServo.attach(busPin);

    clearRegister(false);
    setPeripherals((byte)ARM_L);
    busServo.write(Angles::alStraight);
    pinMode(busPin, OUTPUT);
    delay(500);

    pinMode(busPin, INPUT);
    headServo.write(90);
    armUpperServo.write(0);

    //working startup sequence
    calibrateKill();
    runDiagnostic(LED);
    runDiagnostic(LASER);
    runDiagnostic(BUZZER);
    delay(8000);
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
      armUpperServo.write((float)i / 100.0f * 170);
      if (i == 30 || i == 80) addPeripherals(BUZZ);
      if (i == 40 || i == 90) removePeripherals(BUZZ);

      if (i == 50) addPeripherals(ARM_L);

      busServo.write((float)(i - 50) / 50.0f * 10 + 95);
      delay(4);
    }
    
    for (int i = 0; i < 200; i++) {
      
      if (i < 50) {
        busServo.write((float)(i) / 50.0f * -20 + 105);
        armUpperServo.write((float)(i) / 50.0f * 10 + 170);
    }
      else if 
        ( i < 100){ busServo.write((float)(i - 50) / 50.0f * 20 + 105);
        armUpperServo.write((float)(i - 50) / 50.0f * -10 + 180);
      } else if (i < 150) {
        busServo.write((float)(i - 100) / 50.0f * -20 + 105);
        armUpperServo.write((float)(i - 100) / 50.0f * 10 + 170);
      } else {
        busServo.write((float)(i - 150) / 50.0f * 20 + 105);
        armUpperServo.write((float)(i - 150) / 50.0f * -10 + 180);
      }
      delay(5);
    }
  }

  void orbit() {
    runDiagnostic(CAPACITIVE_SENSOR);
  }

}
