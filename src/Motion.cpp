#include "Motion.h"
#include <Arduino.h>

//All in degrees
    bool lastAngleGreater = false;
    float lastDampedHarmonicValue = 0.0f;

    Motion::Motion(Servo& servo, int angle, float speedTime, motion_t mode)
        : servoReference(servo) {
        set(angle, speedTime, mode);
    }

    Motion::Motion(Servo& servo, int angle, float speedTime, motion_t mode, float timeTo90Deg)
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
 
    int Motion::getCurrentAngle() {
        float time = getTimeSinceStart();
        if (mode != SET_SPEED && time >= speedTime) return endAngle;

        switch (mode) {
            case ROBOTIC: //linear interpolation
                return lerp(time / speedTime, startAngle, endAngle) + 0.5f; 
            
            case NATURAL: {  //smooth step, x^2 (3 - 2x)
                float x = time / speedTime;
                float smoothStepValue = x * x * (3.0f - 2.0f * x);
                return lerp(smoothStepValue, startAngle, endAngle) + 0.5f;
            }

            case BOUNCE: {  //bounce with damped harmonic oscillator 1 - exp(-ζωt) * cos(ωt * sqrt(1 - ζ^2))
                const float zeta = 0.56f; //damp strength
                const float omega = 6.0f; //speed
                float x = time / speedTime;
                float dampedHarmonicValue = 1.0f - (float)exp(-zeta * omega * x) * cos(omega * x * sqrt(1.0f - zeta * zeta));
                
                if (dampedHarmonicValue < lastDampedHarmonicValue) lastAngleGreater = true;
                lastDampedHarmonicValue = dampedHarmonicValue;
                return lerp(dampedHarmonicValue, startAngle, endAngle) + 0.5f;
            }

            case SET_SPEED: //constant speed
                int inversionFactor = endAngle > startAngle ? 1 : -1;
                return speedTime * speedMultiplier * time * inversionFactor + startAngle + 0.5f;
                
        }
    }
 
    void Motion::set(int angle, float speedTime, motion_t mode) {
        startTime = micros();
        startAngle = done ? servoReference.read() : lastUpdatedAngle; 
        endAngle = angle;
        this->speedTime = speedTime;
        this->mode = mode;
        lastDampedHarmonicValue = 0.0f;
        lastAngleGreater = false;
        done = false;
    }

    int Motion::update() {
        int angle = getCurrentAngle();
        servoReference.write(angle);
        if (angle == endAngle) {
            if (mode != BOUNCE) {
                done = true;
            } else if (lastAngleGreater) {
                done = true;
            }
        }
        lastUpdatedAngle = angle;
        return angle;
    }

    bool Motion::finished() {
        return done;
    } 

    int Motion::getServoAngle() {
        return servoReference.read();
    }
    