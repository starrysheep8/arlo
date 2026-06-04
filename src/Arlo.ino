#include "Novae.h"


Servo servo;

void setup() {
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Hello, world!");
  Goldstone::proto(9, 8, 6, 7, A0);
  //Goldstone::printPrototype();
  //Goldstone::proto(22, 24, 29, 27, 25, 23);
  //pinMode(A2, INPUT);



//pinMode(14, OUTPUT);


  //GoldstoneOld::protoProto();
}

void loop() {
//    digitalWrite(14, HIGH);
//    delayMicroseconds(25);
//    digitalWrite(14, LOW);
//
//    unsigned long scanStart = micros();
//    unsigned long signalStart = 0;
//
//    bool lastSignal = LOW;
//    for(;;) {
//      bool signal = digitalRead(15);
//      if (signal == HIGH && lastSignal == LOW) signalStart = micros();
//      if (signal == LOW && lastSignal == HIGH && (micros() - signalStart) / 1000000.0f * 340.0f / 2.0f > 0.0f) break;
//      if (micros() - scanStart >= 1000000) {
//        signalStart = micros();
//        break;
//      }
//      lastSignal = signal;
//    }
//    unsigned long signalTime = micros() - signalStart;
//
//   Serial.print(10.0);
//    Serial.print("\t");
//    Serial.print(0.05);
//    Serial.print("\t");
//    Serial.print(signalTime / 1000000.0f * 340.0f / 2.0f);
//    Serial.println("m");
//    delay(100);
//    return;

  GoldstoneOld::readInputs();
  //GoldstoneOld::readInputs();
  //Serial.println(digitalRead(A2));
  //delay(1);
  //Goldstone::orbit();
  //delay(1);
}
