#pragma once
#include <Servo.h>

//All in degrees
class Motion {
    public:
    typedef enum motionMode {ROBOTIC, EASE_OUT, NATURAL, BOUNCE, SET_SPEED} motion_t;
 
    Motion(Servo& servo, int angle, float speedTime, motion_t mode);
    Motion(Servo& servo, int angle, float speedTime, motion_t mode, float timeTo90Deg);

    void set(int angle, float speedTime, motion_t mode);
    int update();
    bool finished();
    void updateUntilFinished();
    int getServoAngle();
    
    private:
    Servo& servoReference;
    int startAngle;
    int endAngle;
    int lastUpdatedAngle;
    float speedTime; //speed in degrees per second, used only when mode is SET_SPEED;
    float speedMultiplier = 1.0f;
    unsigned long startTime;
    motion_t mode;
    bool done;

    float getTimeSinceStart();
    float lerp(float t, float min, float max);
    int getCurrentAngle();
};