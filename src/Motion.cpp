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

    float lerp(float t, float min, float max) {
        return t * (max - min) + min;
    }
 
    float getCurrentAngle() {
        float time = getTimeSinceStart();
        if (mode != SET_SPEED && time >= speedTime) return endAngle;

        switch (mode) {
            case ROBOTIC: //linear interpolation
                return lerp(time / speedTime, startAngle, endAngle); 
            
            case NATURAL: //smooth step, x^2 (3 - 2x)
                float x = time / speedTime;
                float smoothStepValue = x * x * (3 - 2 * x);
                return lerp(smoothStepValue, startAngle, endAngle);

            case BOUNCE: //bounce with damped harmonic oscillator 1 - exp(-ζωt) * cos(ωt * sqrt(1 - ζ^2))
                const float zeta = 0.75f; //damp strength
                const float omega = 7.0f; //speed
                float x = time / speedTime;
                float dampedHarmonicValue = 1.0f - (float)exp(-zeta * omega * x) * cos(omega * x * sqrt(1.0f - zeta * zeta));
                return lerp(dampedHarmonicValue, startAngle, endAngle);

            case SET_SPEED: //constant speed
                return speedTime * speedMultiplier * time + startAngle;
                
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