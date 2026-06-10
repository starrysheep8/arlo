#include "Arlo.h"

namespace Arlo {

    void capacitiveDiag();
    void ledLaserBuzzerDiag(periph_t liteLasrBuzz);

    void setError(arloError errorCode) {

    }

    void runDiagnostic(diagnostic_t diagnosis) {
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

          case ARM_L:

          case BODY:
        
            break;
        }
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


}