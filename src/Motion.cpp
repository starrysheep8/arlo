#include "Motion.h"
#include <Arduino.h>

//All in degrees
 
    Motion::Motion(Servo& servo, float angle, float speedTime, motion_t mode)
        : servoReference(servo) {
        set(angle, speedTime, mode);
    }

    Motion::Motion(Servo& servo, float angle, float speedTime, motion_t mode, float timeTo90Deg)
        : servoReference(servo) {
        const float measuredSpeedConstant = 0.1f; //AHHHHHHHH ACTUALLY MEASURE THIS SPEED CONSTANT, YOU JUST GUESSED THIS NUMBER
        speedMultiplier = measuredSpeedConstant/timeTo90Deg;
        set(angle, speedTime, mode);
    } 

    float Motion::getTimeSinceStart() {
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

    float Motion::lerp(float t, float min, float max) {
        return t * (max - min) + min;
    }
 
    float Motion::getCurrentAngle() {
        float time = getTimeSinceStart();
        if (mode != SET_SPEED && time >= speedTime) return endAngle;

        switch (mode) {
            case ROBOTIC: //linear interpolation
                return lerp(time / speedTime, startAngle, endAngle); 
            
            case NATURAL: {  //smooth step, x^2 (3 - 2x)
                float x = time / speedTime;
                float smoothStepValue = x * x * (3.0f - 2.0f * x);
                return lerp(smoothStepValue, startAngle, endAngle);
            }

            case BOUNCE: {  //bounce with damped harmonic oscillator 1 - exp(-ζωt) * cos(ωt * sqrt(1 - ζ^2))
                const float zeta = 0.75f; //damp strength
                const float omega = 7.0f; //speed
                float x = time / speedTime;
                float dampedHarmonicValue = 1.0f - (float)exp(-zeta * omega * x) * cos(omega * x * sqrt(1.0f - zeta * zeta));
                return lerp(dampedHarmonicValue, startAngle, endAngle);
            }

            case SET_SPEED: //constant speed
                return speedTime * speedMultiplier * time + startAngle;
                
        }
    }
 
    void Motion::set(float angle, float speedTime, motion_t mode) {
        startTime = micros();
        startAngle = done ? servoReference.read() : lastUpdatedAngle; 
        endAngle = angle;
        this->mode = mode;
        done = false;
    }

    float Motion::update() {
        float angle = getCurrentAngle();
        servoReference.write(angle);
        lastUpdatedAngle = angle;
        if (angle == endAngle) done = true;
        return angle;
    }

    bool Motion::finished() {
        return done;
    } 

    int Motion::getServoAngle() {
        return servoReference.read();
    }
    