#pragma once
#include <Servo.h>

//All in degrees
class Motion {
    public:
    typedef enum motionMode {ROBOTIC, NATURAL, BOUNCE, SET_SPEED} motion_t;
 
    Motion(Servo& servo, float angle, float speedTime, motion_t mode);
    Motion(Servo& servo, float angle, float speedTime, motion_t mode, float timeTo90Deg);

    void set(float angle, float speedTime, motion_t mode);
    float update();
    bool finished();
    int getServoAngle();
    
    private:
    Servo& servoReference;
    float startAngle;
    float endAngle;
    float lastUpdatedAngle;
    float speedTime; //speed in degrees per second, used only when mode is SET_SPEED;
    float speedMultiplier = 1.0f;
    unsigned long startTime;
    motion_t mode;
    bool done;

    float getTimeSinceStart();
    float lerp(float t, float min, float max);
    float getCurrentAngle();
};