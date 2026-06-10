#include "Arlo.h"

void setup() {
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Hello, world!");
  Arlo::proto(9, 8, 6, 7, A0);
}

void loop() {
  Arlo::orbit();
}
