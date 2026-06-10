#include "Arlo.h"

namespace Arlo {
    int killNoTouchBaseline;
    int killTouchBaseline;
    int killHighSubtractor = 1000; //randomly analogReading kill will output a regularvalue plus 1000 exactly, this accounts for that when it occurs


    bool getPeripheral(periph_t peripheral) {
        return latchBits & peripheral;
    }

    void setPeripherals(byte bits) {
        //validate & set bus
        bool setOutputAfterLatch = false;
        bus_t desiredBus = getBusRequirements(bits);
        if (bits == BODY) Serial.println(desiredBus);
        switch (desiredBus) {
          case BUS_ERROR:
            setError(BUS_INPUT_OUTPUT_SET);
            return;

          case BUS_INPUT:
            pinMode(busPin, INPUT);
            break;

          case BUS_OUTPUT:
              pinMode(busPin, INPUT);
              setOutputAfterLatch = true;
            break;

          case NO_BUS:
            pinMode(busPin, INPUT);
            break;
        }

        //set peripherals if bus was valid
        shiftIn(bits);
        //set latch
        flipMsb();
        flipLsb();
        latchBits = registerBits;
        if (setOutputAfterLatch) {
          pinMode(busPin, OUTPUT);
        }
    }  

    void addPeripherals(byte bits) {setPeripherals(bits | latchBits);}

    void removePeripherals(byte bits) {setPeripherals(~bits & latchBits);}

    bus_t getBusRequirement(arloPeripheral peripheral) { //-1: bus not used, OUTPUT, INPUT
        bus_t busState = NO_BUS;
        if (peripheral & BUS_OUTPUTS == true)
          busState = BUS_OUTPUT;
        else if (peripheral & BUS_INPUTS == true) 
          busState = BUS_INPUT;
        
        return busState;
    }

    bus_t getBusRequirements(byte peripheralBits) { //-1: bus not used, OUTPUT, INPUT, -2 error
      bool isOutput = false;
      bool isInput = false;
      for (int i = 0; i < 8; i++) {
        periph_t currentPeriph = (periph_t)(peripheralBits & (1 << i));
        if (getBusRequirement(currentPeriph) == BUS_OUTPUT) isOutput = true; 
        if (getBusRequirement(currentPeriph) == BUS_INPUT) isInput = true; 
      }
      
      if (isOutput && isInput) {
        return BUS_ERROR;
      } else if (isOutput) {
        return BUS_OUTPUT;
      } else if (isInput) {
        return BUS_INPUT;
      } else return NO_BUS;
    }
  
    bus_t getCurrentBusState() {
      bus_t busState = getBusRequirements(latchBits);
      if (busState == BUS_ERROR) {
        setError(BUS_INPUT_OUTPUT_SET);
      }
      return busState;
    }
  
    kill_t getKillState() {
      int doubleTries = 1; //averages data from (2 * doubleTries + 1) sensor reads
      int total = 0;
  
      if (getKillStateRaw(true) == TOUCH) total++;
      for (int i = 0; i < doubleTries; i++) {
        if (getKillStateRaw() == TOUCH) total++;
      }
      return total >= doubleTries + 1 ? TOUCH : NO_TOUCH;
    }
  
    kill_t getKillStateRaw(bool trySetKillPeripherial) { 
      if (!getPeripheral(KILL))
        return NOT_SENSING;
  
      if (trySetKillPeripherial) {
        if (getBusRequirements(latchBits | KILL) == BUS_ERROR)
          return NOT_SENSING;
        else
          addPeripherals(KILL);
      }
  
      
      int sensorValue = analogRead(busPin);
      if (sensorValue > killHighSubtractor) sensorValue -= killHighSubtractor;
      return sensorValue > killTouchBaseline ? TOUCH : NO_TOUCH;
    }
  
    void calibrateKill() {
        setPeripherals(KILL);
        int sampleCount = 10;
        int samples[sampleCount];
        for (int i = 0; i < sampleCount; i++) {
          delay(5);
          samples[i] = analogRead(busPin);
        }
    
        int mode = getMode(samples, sampleCount);
        killNoTouchBaseline = mode;
        killTouchBaseline = mode + 9;
    }
  
    bool arrayContains(int element, int array[], int size) {
        for (int i = 0; i < size; i++) {
            if (array[i] == element) return true;
        }
        return false;
    }

    int getMode(int intArray[], int size) {
        //get list of unique elements in intArray
        int uniqueElementsFound = 0;
        int uniqueElements[size];
        for (int i = 0; i < size; i++) {
            if (!arrayContains(intArray[i], uniqueElements, uniqueElementsFound)) {
                uniqueElements[uniqueElementsFound] = intArray[i];
                uniqueElementsFound++;
            }
        }

        //count occurances of each unique element in intArray
        int foundCounts[uniqueElementsFound];
        for (int i = 0; i < uniqueElementsFound; i++) {foundCounts[i] = 0;}

        for (int i = 0; i < uniqueElementsFound; i++) {
            int currentElement = uniqueElements[i];
            for (int j = 0; j < size; j++) {
                if (intArray[j] == currentElement) {
                    foundCounts[i]++;
                }
            }
        }

        //find and return the element that occurred the most
        int biggestIndex = 0;
        int biggestCount = 0;
        for (int i = 0; i < uniqueElementsFound; i++) {
            if (foundCounts[i] > biggestCount) {
                biggestCount = foundCounts[i];
                biggestIndex = i;
            }
        }

        return uniqueElements[biggestIndex];
    }
}