#include "../Arlo.h"

namespace Arlo {
    const uint16_t NO_SCAN = -1;
    const uint16_t MAX_SCAN = -2;
    //AHHHHHHHH UPDATE THIS VALUE EXPERIMENTALLY
    const uint16_t MAX_SONAR_MICROS = 46647;

    const int scanTries = 3;
    const unsigned long timeoutMillis = 60;

    const int trackedAngleCount = 360;
    uint16_t scanBuffer[trackedAngleCount];
    uint16_t space[trackedAngleCount];

    uint16_t scanRaw() {
        if (getPeripheral(TRIG)) {
            removePeripherals(TRIG | ECHO);
        }

        bool echoWasHigh = false;
        addPeripherals(TRIG);
        delayMicroseconds(10);
        unsigned long scanStart = micros();
        setPeripherals((latchBits ^ TRIG) | ECHO);
int tries = 0;
        for (unsigned long currentTime = micros(); currentTime - scanStart < timeoutMillis * 1000UL; currentTime = micros()) {
            Serial.println(tries++);
            if (digitalRead(busPin) == LOW) {
                if (echoWasHigh) {
                    uint16_t deltaScanTime = currentTime - scanStart;
                    return deltaScanTime >= MAX_SONAR_MICROS ? MAX_SCAN : deltaScanTime;
                }
            } else echoWasHigh = true;
        }
        
        return NO_SCAN;
    }

    int scanTime() {  //returns time [μs]; updates tracked angles & space

    }
    
    float scan() {  // returns distance [m]; updates tracked angles & space
    
    }
    //ahhhhhhh if something is varrying on the initial scan, mark it and come back to it later. If it continues to vary in the same way, it's part of the space, otherwise it is left blank
    void initializeScanArrays() {
        for (int i = 0; i < trackedAngleCount; i++) {
            scanBuffer[i] = NO_SCAN;
            space[i] = NO_SCAN;
        }
    }

    float establishSpace() { //overwrites and initializes the space
        initializeScanArrays();
        for (int i = 0; i < 360; i++) {
            headMotion.set(i, 0.1f, Motion::ROBOTIC);
            do {
                headMotion.update();
            } while(headMotion.finished() == false);
            scanRaw();
            delay(10);
            scanRaw();
            delay(10);
            Serial.println(scanRaw());
        }
    }
    
    float getSpaceIntegrity() { //how much of the space is filled out

    }
    
    vec2_t getSpaceCenter() {
    
    }


}