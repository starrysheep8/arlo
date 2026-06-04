#include "Novae.h"

#define STEPPER_DELAY 3000 //μs
#define STEP2RAD 0.0031463f
#define TIME2METER 0.00017f
#define METER2TIME 5882.352941f

using namespace cmb;

namespace GoldstoneOld {
  Servo mount;

  uint8_t trig;
  uint8_t echo;
  uint8_t steps[4];

  static uint8_t currentStep = 0;
  static int rawAngle = 0;

  unsigned long sineTime = 0;

  //eyes
  constexpr int scanRange = 400;
  constexpr int scanSize = scanRange * 2 + 1; //size of the array
  int anomalyRange = 40;
  unsigned int scanValues[scanSize];

  void proto(uint8_t trigger, uint8_t echo, uint8_t step1, uint8_t step2, uint8_t step3, uint8_t step4) {

    trig = trigger;
    GoldstoneOld::echo = echo;
    steps[0] = step1; steps[1] = step2; steps[2] = step3; steps[3] = step4;

    pinMode(trigger, OUTPUT);
    pinMode(echo, INPUT);
    for (int i = 0; i < 4; i++)
      pinMode(steps[i], OUTPUT);

    //calibrate head
    //step(1000);
    //step(-900);

  }

  void orbit() {
    unsigned long milliseconds = millis();
    static unsigned long lastMillis = milliseconds;
    static unsigned long deltaTime;
    deltaTime = milliseconds - lastMillis;
    lastMillis = milliseconds;

    sineTime += deltaTime;

    int specificStep = scanRange + scanRange * sin(6.2831853072f / 6000.0f * sineTime);
    setStep(specificStep);
    scanValues[specificStep] = scan();
    printData(specificStep, scanValues[specificStep]);
  }

  void setStep(int toStep) {
    step(toStep - rawAngle);
  }

  void step(int stepCount) {
    rawAngle += stepCount;
    bool CW = (stepCount >= 0);
    for (int i = 0; i < 4; i++) 
      digitalWrite(i, LOW);
    if (CW) {
      for (int i = 0; i < stepCount; i++) {
        currentStep = ++currentStep % 4;
        digitalWrite(steps[currentStep], HIGH);
        delayMicroseconds(STEPPER_DELAY );
        digitalWrite(steps[currentStep], LOW);
      }
    } else {
      for (int i = 0; i > stepCount; i--) {
        currentStep = --currentStep % 4;
        digitalWrite(steps[currentStep], HIGH);
        delayMicroseconds(STEPPER_DELAY);
        digitalWrite(steps[currentStep], LOW);
      }
    }
  }

  unsigned int scan() {
    digitalWrite(trig, HIGH);
    delayMicroseconds(25);
    digitalWrite(trig, LOW);

    unsigned long scanStart = micros();
    unsigned long signalStart = 0;

    bool lastSignal = LOW;
    for(;;) {
      bool signal = digitalRead(echo);
      if (signal == HIGH && lastSignal == LOW) signalStart = micros();
      if (signal == LOW && lastSignal == HIGH && (micros() - signalStart) / 1000000.0f * 340.0f / 2.0f > 0.0f) break;
      if (micros() - scanStart >= 100000) {
        signalStart = micros();
        break;
      }
      lastSignal = signal;
    }
    unsigned long signalTime = micros() - signalStart;
    return signalTime;
  }

  void printData(int rawAngle, unsigned int rawRadius) {
    //convert polar coordinates to Euclidean
    float th = (rawAngle - scanRange) * STEP2RAD - PI/2.0;
    float r = rawRadius;
    float worldX = TIME2METER * r*cos(th);
    float worldY = TIME2METER * r*sin(th);

    Serial.print(worldX);
    Serial.print(" ");
    Serial.print(worldY);
    Serial.print("\n");
    return;
  }


//PROTOTYPE
int busPin = A0;
int demulAPin = 9;
int demulBPin = 8;
int armUPin = 7;
int armLPin = 6;

bool busMode = INPUT;
int pwm = 90; // or 150
byte registerValue = 0;
byte latchedValue = 0;
bool latched = false;
bool justLatched = false;
bool demulBitA = false;
bool demulBitB = false;
bool serialBit = false;
float scannedMeters = 0.0f;
bool killState = false;

Servo neck;
Servo drain;
Servo armU;
Servo armL;

int drainPwm = -1;

bool tryTrigger = false;

String peripherals[8] = {"HEAD", "ECHO", "KILL", "BODY", "LITE", "LASR", "BUZZ", "TRIG"};

void protoProto() {
  neck.attach(busPin);
  drain.attach(3);
  armU.attach(armUPin);
  armL.attach(armLPin);
  pinMode(busPin, busMode);
  pinMode(demulAPin, OUTPUT);
  pinMode(demulBPin, OUTPUT);
  clearRegister();
  //delayMicroseconds(1);
  digitalWrite(demulAPin, HIGH);
  Serial.print("\n\n\n");
}

void readInputs() {
  if (drainPwm != -1) {
    drain.write(drainPwm++);
    drainPwm %= 90;
  }
  bool updateStates = false;
  char readByte = '\0';
  if (Serial.available() > 0)
    readByte = Serial.read();
  //process input
  switch (readByte) {
    case 's': case '2':
      demulBitA = !demulBitA;
      updateStates = true;
      break;
    case 'a': case '1':
      demulBitB = !demulBitB;
      updateStates = true;
      break;
    case 'p':
      pwm = pwm == 45 ? 135 : 45;
      updateStates = true;
      break;
    case 'b':
      busMode = !busMode;
      updateStates = true;
      break;
    case 't':
      tryTrigger = true;
      updateStates = true;
      break;
    case 'd':
      break;
      if (drainPwm == -1) drainPwm = 0;
      else drainPwm = -1;
      updateStates = true;
      break;
    case 'u':
      armU.write(pwm);
      updateStates = true;
      break;
    case 'l':
      armL.write(pwm);
      updateStates = true;
      break;
    case 'c':
      clearRegister();
      latched = false;
      registerValue = 0b00000000;
      demulBitA = true;
      demulBitB = false;
      serialBit = false;
      updateStates = true;
      break;
  }
  if (updateStates == true) {
    if (tryTrigger) {
      digitalWrite(demulBPin, LOW);
    //delayMicroseconds(1);
      digitalWrite(demulAPin, LOW);

    //delayMicroseconds(1);
      digitalWrite(demulAPin, HIGH);
    //delayMicroseconds(1);
      digitalWrite(demulBPin, HIGH);
        for (int i = 0; i < 7; i++) {
          //delayMicroseconds(1);
            digitalWrite(demulBPin, LOW);
          //delayMicroseconds(1);
            digitalWrite(demulBPin, HIGH);
        }
      //delayMicroseconds(1);
        digitalWrite(demulAPin, LOW);

        delayMicroseconds(25);
          digitalWrite(demulBPin, LOW);
        //delayMicroseconds(1);
          digitalWrite(demulAPin, HIGH);
        //delayMicroseconds(1);
          digitalWrite(demulBPin, HIGH);
        //delayMicroseconds(1);
          digitalWrite(demulBPin, LOW);
        //delayMicroseconds(1);
          digitalWrite(demulBPin, HIGH);

        //delayMicroseconds(1);
          digitalWrite(demulAPin, LOW);
          latched = false;
          latchedValue = 0b10000000;
          registerValue = 0b00000010;
          demulBitA = true;
          demulBitB = false;
          serialBit = false;
      tryTrigger = false;
    }

    processStateData();
    printPrototype();
  }
}

void processStateData() {
  pinMode(busPin, busMode);
  digitalWrite(demulAPin, !demulBitA);
  digitalWrite(demulBPin, !demulBitB);

  if (busMode == OUTPUT) neck.write(pwm);

  if (!demulBitB && !demulBitA) serialBit = false;  //00 R
  if (demulBitB && demulBitA) serialBit = true;     //11 S
  if (!latched && !demulBitB && demulBitA)          //01 RCLK
    justLatched = true;
  latched = (!demulBitB && demulBitA);
  if (demulBitB && !demulBitA) {                    //10 SRCLK
    registerValue <<= 1;
    registerValue |= serialBit;
  }

  scannedMeters = 0.0f;
  if (justLatched) {
    bool listenForEcho = true;                                            //listen for echo if...
    if ((latchedValue & 0b10000000) != 0b10000000) listenForEcho = false; //trig was last high
    if ((registerValue & 0b10000010) != 0b00000010) listenForEcho = false;//trig is low and echo is high
    if (busMode != INPUT) listenForEcho = false;                          //bus is listening for input

    latchedValue = registerValue;
    if (listenForEcho) {//===================== listening for echo ===========================
      unsigned long scanStart = micros();
      unsigned long signalStart = 0;

      bool lastSignal = LOW;
      for(;;) {
        bool signal = digitalRead(busPin);
        //Serial.println(analogRead(busPin));
        if (signal == HIGH && lastSignal == LOW) signalStart = micros();
        if (signal == LOW && lastSignal == HIGH && (micros() - signalStart) > 5) break;
        if (micros() - scanStart >= 100000) {
          signalStart = micros();
          break;
        }
        lastSignal = signal;
      }
      unsigned long signalTime = micros() - signalStart;
      Serial.println(signalTime);
      scannedMeters = TIME2METER * signalTime;
    }//======================================== listening for echo ===========================
    justLatched = false;
  }
  if (busMode == INPUT && (latchedValue & 0b00000100) == 0b00000100) killState = digitalRead(busPin);
}

void printPrototype() {
  Serial.print("\033[1A\033[1A\033[1A\033[1A\r");
  Serial.print("Bus Mode: ");
  Serial.print(busMode == INPUT ? "INPUT" : "OUTPUT");
  if (busMode == INPUT) {
    if ((latchedValue & 0b00000100) == 0b00000100) {
      Serial.print(", "); Serial.print(killState ? "KILL" : "SAFE");
    }
    if ((latchedValue & 0b10000010) == 0b00000010) {
      Serial.print(", "); Serial.print(scannedMeters); Serial.print("m");
    }
  } else {
    Serial.print(", pwm: "); Serial.print(pwm);
  }
  Serial.println("\033[K");

  Serial.print("Demul: "); Serial.print(demulBitB); Serial.print(demulBitA);
  Serial.print(", SBIT: "); Serial.print(serialBit);
  if (latched) Serial.print(", LATCHED");
  Serial.println("\033[K");

  Serial.print("Latch: "); for (int i = 7; i >= 0; i--) Serial.print(bitRead(latchedValue, i));
  Serial.print(", Register: "); for (int i = 7; i >= 0; i--) Serial.print(bitRead(registerValue, i));
  Serial.println("\033[K");

  for (int i = 0; i < 8; i++) {
    if (latchedValue >> i & 1) {
      Serial.print(peripherals[i]);
    } else Serial.print("____");
    
    if (i != 7) Serial.print(", ");
  }
  Serial.println("\033[K");
}

void clearRegister() {
  digitalWrite(demulAPin, HIGH);
  //delayMicroseconds(1);
  digitalWrite(demulBPin, HIGH);
  for (int i = 0; i < 8; i++) {
      //delayMicroseconds(1);
      digitalWrite(demulBPin, LOW);
      //delayMicroseconds(1);
      digitalWrite(demulBPin, HIGH);
  }
  //delayMicroseconds(1);
  digitalWrite(demulAPin, LOW);
}

}