#include "Arlo.h"

namespace Arlo {

    void capacitiveDiag();
    void ledLaserBuzzerDiag(periph_t liteLasrBuzz);
    void servoDiag(Motion servoMotion);

    void setError(arloError errorCode) {

    }

    void runDiagnostic(diagnostic_t diagnosis) {
        byte initialPeripherals = latchBits;
        setPeripherals(0);

        switch (diagnosis) {
          case TRIG_ECHO:
            break;

          case CAPACITIVE_SENSOR: {
            capacitiveDiag();
            break;
          }

          case LED:
            ledLaserBuzzerDiag(LITE);
            break;
        
          case LASER:
            ledLaserBuzzerDiag(LASR);
            break;
        
          case BUZZER:
            ledLaserBuzzerDiag(BUZZ);
            break;

          case SERVO_ARM_L: {
            int initialArmUpperAngle = armUpperMotion.getServoAngle();
            armUpperMotion.set(90, 0.5f, Motion::ROBOTIC);
            while (!armUpperMotion.finished()) {armUpperMotion.update();}
            setPeripherals(ARM_L);
            servoDiag(armLowerMotion);
            while(armLowerMotion.finished() == false) {armLowerMotion.update();}
            armUpperMotion.set(0, 0.5f, Motion::ROBOTIC);
            while(armUpperMotion.finished() == false) {armUpperMotion.update();}
            break;
          }

          case SERVO_BODY:
            setPeripherals(BODY);
            servoDiag(bodyMotion);
            while(bodyMotion.finished() == false) {bodyMotion.update();}
            break;
            
          case SERVO_ARM_U:
            servoDiag(armUpperMotion);
            break;

          case SERVO_HEAD:
            servoDiag(headMotion);
            break;
        }

        setPeripherals(initialPeripherals);
    }    

    void capacitiveDiag() {
        setPeripherals(KILL | LITE | LASER);
        const unsigned long timeoutSeconds = 8;
        unsigned long timeoutTimestamp = millis() + 1000 * timeoutSeconds;
        bool diagnosticResult = false;
        delay(10);

        while (millis() < timeoutTimestamp) { //wait for sensor touch
            if (getKillState() == TOUCH) {
            addPeripherals(BUZZ);
            removePeripherals(LITE | LASER);
            timeoutTimestamp = millis() + 1000 * timeoutSeconds;

            while (millis() < timeoutTimestamp) { //wait for sensor touch to stop
                if (getKillState() == NO_TOUCH) {
                diagnosticResult = true;
                timeoutTimestamp = 0;
                }
            }
            }
        } 

        if (diagnosticResult == false) {
            setError(NEGATIVE);
        } else {
            setPeripherals(LITE | LASER);
            delay(50);
            setPeripherals(0);
            delay(50);
            setError(POSITIVE);
        }
        setPeripherals(0);
    }

    void ledLaserBuzzerDiag(periph_t liteLasrBuzz) {
        //make sure only LITE, LASR, or BUZZ peripherals were passed
        byte unrelatedPeripheralBits = liteLasrBuzz & ~(LITE | LASR | BUZZ);
        if (unrelatedPeripheralBits != 0) {
            setError(BAD_PERIPHERAL);
            return;
        }
        setPeripherals(liteLasrBuzz);           
        delay(200);
        setPeripherals(0);
        for (int i = 0; i < 2; i++) {
            delay(50);
            setPeripherals(liteLasrBuzz);
            delay(50);
            setPeripherals(0);
        }
    }

    void servoDiag(Motion servoMotion) {
        //initialize the angle
        const float testTime = 1.0f;
        const int intermittentMillis = 450;
        int initialAngle = servoMotion.getServoAngle();
        servoMotion.set(0, testTime, Motion::ROBOTIC);
        while(servoMotion.finished() == false) {servoMotion.update();}
        delay(intermittentMillis + 70); 

        
        servoMotion.set(180, testTime, Motion::ROBOTIC);        
        while(servoMotion.finished() == false) {servoMotion.update();}
        addPeripherals(BUZZ | LITE);
        delay(70);
        removePeripherals(BUZZ | LITE);
        delay(intermittentMillis);

        servoMotion.set(0, testTime, Motion::NATURAL);        
        while(servoMotion.finished() == false) {servoMotion.update();}
        addPeripherals(BUZZ | LITE);
        delay(70);
        removePeripherals(BUZZ | LITE);
        delay(intermittentMillis);

        servoMotion.set(150, testTime, Motion::BOUNCE);        
        while(servoMotion.finished() == false) {servoMotion.update();}
        addPeripherals(BUZZ | LITE);
        delay(70);
        removePeripherals(BUZZ | LITE);
        delay(intermittentMillis);

        servoMotion.set(0, 180.0f, Motion::SET_SPEED);        
        while(servoMotion.finished() == false) {servoMotion.update();}
        addPeripherals(BUZZ | LITE);
        delay(70);
        removePeripherals(BUZZ | LITE);
        delay(intermittentMillis);

        //set angle back to how it started
        servoMotion.set(initialAngle, 0.5f, Motion::ROBOTIC);
        while(servoMotion.finished() == false) {servoMotion.update();}
        addPeripherals(BUZZ | LITE);
        delay(300);
        removePeripherals(BUZZ | LITE);
    }

}