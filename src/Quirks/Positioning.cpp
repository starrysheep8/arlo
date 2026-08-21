#include "../Arlo.h"

namespace Arlo {
    const uint16_t NO_SCAN = -1;
    const uint16_t MAX_SCAN = -2;
    //AHHHHHHHH UPDATE THIS VALUE EXPERIMENTALLY
    const uint16_t MAX_SONAR_MICROS = 46647;
    const uint16_t MAX_AVERGAE_VARIATION = 575;
    const float MAX_RANGE_VARIATION = 0.08f;
    const float SUSPICION_VARIATION = 0.05f; // suspicious if range is greater than SUSPICION_VARIATION * average

    const int scanTries = 3;
    const unsigned long timeoutMillis = 100;

    const int trackedAngleCount = 360;
    uint16_t scanBuffer[trackedAngleCount];
    uint16_t space[trackedAngleCount];

    uint16_t scanRaw();

    void populateScanArray(uint16_t* scanArray) {
        for (int i = 0; i < scanTries; i++) {
            scanArray[i] = scanRaw();
            if (i + 1 < scanTries) {
                delay(timeoutMillis - scanArray[i] / 1000UL); //wait for previous trig signal to be gone from the room before sending another
            }
        }
    }

    uint16_t scanRaw() {
        if (getPeripheral(TRIG) || true) {
            removePeripherals(TRIG | ECHO);
        }

        bool echoWasHigh = false;
        addPeripherals(TRIG);
        delayMicroseconds(10);
        unsigned long scanStart = micros();
        setPeripherals((latchBits ^ TRIG) | ECHO);
        
        for (unsigned long currentTime = micros(); currentTime - scanStart < timeoutMillis * 1000UL; currentTime = micros()) {
            bool state = digitalRead(busPin);
            if (state == LOW) {
                if (echoWasHigh) {
                    unsigned long deltaScanTime = currentTime - scanStart;
                    if (deltaScanTime <= 5) { //reject short pulses less than like 1cm away
                        echoWasHigh = false;
                        continue;
                    }
                    return (uint16_t)(deltaScanTime >= MAX_SONAR_MICROS ? MAX_SCAN : deltaScanTime);
                }
            } else {
                if (echoWasHigh == false) scanStart = micros();
                echoWasHigh = true;
            }
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

    uint16_t getScanAverage(uint16_t* scanArray) {
        uint32_t total = 0;
        for (int i = 0; i < scanTries; i++) {
            total += scanArray[i];
        }
        return (uint16_t)(total / scanTries);
    }

    uint16_t getScanRange(uint16_t* scanArray) {
        uint16_t max = 0;
        uint16_t min = -1;
        for (int i = 0; i < scanTries; i++) {
            if (scanArray[i] > max) max = scanArray[i];
            if (scanArray[i] < min) min = scanArray[i];
        }
        return max - min;
    }

    float establishSpace() { //overwrites and initializes the space
        
        initializeScanArrays();
        uint16_t currentScans[scanTries];
        uint16_t** suspiciousScans = new uint16_t*[trackedAngleCount];

        for (int theta = 0; theta < trackedAngleCount; theta++) { //initial pass
            headMotion.set((float)theta + 0.5f / 360.0f, 0.02f, Motion::ROBOTIC); //AHHHHHHH update this to account for body rotation
            headMotion.updateUntilFinished();

            //gather initial scans
            for (int i = 0; i < trackedAngleCount; i++) {suspiciousScans[i] = nullptr;}
            populateScanArray(currentScans);

            //determine if initial scans are confident or suspicious
            uint16_t average = getScanAverage(currentScans);
            uint16_t range = getScanRange(currentScans);
            if (average * SUSPICION_VARIATION > range) { //scan was suspicious; save it for later
                uint16_t* scanCopy = new uint16_t[scanTries];
                for (int i = 0; i < scanTries; i++) {scanCopy[i] = currentScans[i];} 
                suspiciousScans[theta] = scanCopy;
                space[theta] = NO_SCAN;
            } else { //scan is confident, save only its average as a baseline
                space[theta] = average;
            }
            scanBuffer[theta = average];

        } //end initial pass

        headMotion.set(0, 0.5f, Motion::EASE_OUT); //AHHHHHHHH update this to account for body rotation also
        headMotion.updateUntilFinished();
        
        for (int theta = 0; theta < trackedAngleCount; theta++) { //second pass
           if (space[theta] != NO_SCAN) {continue;} //skip confident scans

            uint16_t deltaTheta = theta - headMotion.getServoAngle();
            headMotion.set((float)theta + 0.5f / 360.0f, 0.02f * deltaTheta, Motion::ROBOTIC); //AHHHHHHH update this to account for body rotation
            headMotion.updateUntilFinished();
            
            populateScanArray(currentScans);
            
            uint16_t average = getScanAverage(currentScans);
            uint16_t range = getScanRange(currentScans);
            
            uint16_t firstAverage = getScanAverage(suspiciousScans[theta]);
            uint16_t firstRange = getScanRange(suspiciousScans[theta]);

            //if the suspicious angle is acting similarly to how it did the first time, it is no longer suspicious so add it to the space
            if (average > firstAverage + MAX_AVERGAE_VARIATION / 2 ||
                average < firstAverage - MAX_AVERGAE_VARIATION / 2 ||
                range > firstRange + firstRange * MAX_RANGE_VARIATION / 2 ||
                range < firstRange - firstRange * MAX_AVERGAE_VARIATION / 2) {
                    
                    space[theta] = average;
                } else {
                    space[theta] = NO_SCAN;
                }
            
            scanBuffer[theta] = average;
        } //end second pass

        return getSpaceIntegrity();
    }
    

    float getSpaceIntegrity() { //how much of the space is filled out
        int spaceAnglesFilled = 0;
        for (int i = 0; i < trackedAngleCount; i++) {
            if (space[i] != NO_SCAN) spaceAnglesFilled++;
        }
        return (float)spaceAnglesFilled / trackedAngleCount;
    }
    
    vec2_t getSpaceCenter() {
    
    }


}