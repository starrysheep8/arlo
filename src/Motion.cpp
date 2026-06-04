#include <Arduino.h>
#include <Servo.h>

//All in degrees
class Motion {
    public:
    typedef enum motionMode {ROBOTIC, NATURAL, BOUNCE, SET_SPEED} motion_t;
 
    Motion(Servo servo, float angle, float speedTime, motion_t mode) {
        servoReference = servo;
        set(angle, speedTime, mode);
    }

    Motion(Servo servo, float angle, float speedTime, motion_t mode, float timeTo90Deg) {
        servoReference = servo;
        const float measuredSpeedConstant = 0.1f; //AHHHHHHHH ACTUALLY MEASURE THIS SPEED CONSTANT, YOU JUST GUESSED THIS NUMBER
        speedMultiplier = measuredSpeedConstant/timeTo90Deg;
        set(angle, speedTime, mode);
    } 

    private:
    Servo servoReference;
    float startAngle;
    float endAngle;
    float lastUpdatedAngle;
    float speedTime; //speed in degrees per second, used only when mode is SET_SPEED;
    float speedMultiplier = 1.0f;
    unsigned long startTime;
    motion_t mode;
    bool done;

    float getTimeSinceStart() {
        unsigned long currentTime = micros();
        float timeSinceStart = 0.0f;
        if (currentTime >= startTime) {
           timeSinceStart = ((float)(currentTime - startTime));
        } else { //account for possible overflow
            const unsigned long maxTimeBeforeOverflow = -1UL;
            timeSinceStart = maxTimeBeforeOverflow - startTime;
            timeSinceStart += currentTime;
        }

        return timeSinceStart /= 1000000.0f; //convert from microseconds to seconds 
    }
 
    float getCurrentAngle() {
        switch (mode) {
            case ROBOTIC:
               break;
            
            case NATURAL:
                break;

            case BOUNCE:
                break;

            case SET_SPEED:
                break;
        }
    }
 
    public:
    void set(float angle, float speedTime, motion_t mode) {
        startTime = micros();
        startAngle = done ? servoReference.read() : lastUpdatedAngle; 
        endAngle = angle;
        this->mode = mode;
        done = false;
    }

    float update() {
        float angle = getCurrentAngle();
        servoReference.write(angle);
        lastUpdatedAngle = angle;
        if (angle == endAngle) done = true;
        return angle;
    }

    bool finished() {
        return done;
    } 
    
};