#include <Arduino.h>
#include <Servo.h>

//0: GND, 00: 5V, 000: 3V3
namespace cmb {
  typedef struct Vector2 {
    float x, y;
  } V2_t;

  typedef struct Vector2I {
    int x, y;
  } V2I_t;

  const float deg2rad = 0.01745329252;
  const float rad2deg = 57.2957795131;

  int getMode(int intArray[], int size);
  bool arrayContains(int element, int array[], int size);
}

namespace Constelation {
  void proto(uint8_t pwmPin);
  void orbit();
  void set(bool state);
  void unset();
};

namespace Webb { //1
  void proto(uint8_t pin, unsigned timeoutSeconds);
  void orbit();
  bool state();
  unsigned long timeSinceDetection();
}

namespace Kepler { //2
  enum lightLevel : uint8_t {dark, dim, bright};
  typedef enum lightLevel level_t;

  void proto(uint8_t analogPin);
  void orbit();
  float getAverage();
  level_t getLightLevel();
}

namespace Goldstone {
  void proto(uint8_t pinTX_plus, uint8_t pinTX_minus, uint8_t pwmRX_plus, uint8_t pwmRX_minus, uint8_t pwmSBU);
  void orbit();
  //void step(int stepCount);
  //void setStep(int toStep);
  //unsigned int scan();
  //void printData(int rawAngle, unsigned int rawRadius);
  //void protoProto();
  //void readInputs();
  //void processStateData();
  //void printPrototype();
  //void clearRegister();
}


namespace GoldstoneOld {
  void step(int stepCount);
  void setStep(int toStep);
  unsigned int scan();
  void printData(int rawAngle, unsigned int rawRadius);
  void protoProto();
  void readInputs();
  void processStateData();
  void printPrototype();
  void clearRegister();
}