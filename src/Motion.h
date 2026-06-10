
#include <Arduino.h>
#include <Servo.h>

//All in degrees
class Motion {
    typedef enum motionMode {ROBOTIC, NATURAL, BOUNCE, SET_SPEED} motion_t;
    
    Motion(Servo servo, float angle, float speedTime, motion_t mode);
    Motion(Servo servo, float angle, float speedTime, motion_t mode, float timeTo90Deg);

    void set(float angle, float speedTime, motion_t mode);
    float update();
    bool finished();
};