#include "Arlo.h"

namespace Arlo {
    uint8_t demulLsbPin;
    uint8_t demulMsbPin;
    uint8_t armUpperPin;
    uint8_t headPin;
    uint8_t busPin;

    bool demulLsb;
    bool demulMsb;
    byte registerBits;
    byte latchBits;

    const int holdMicros = 3;

    void setLsb(bool state) {demulLsb = state; digitalWrite(demulLsbPin, !state);}
    void setMsb(bool state) {demulMsb = state; digitalWrite(demulMsbPin, !state);}
    void flipLsb() {demulLsb = !demulLsb; digitalWrite(demulLsbPin, !demulLsb);}
    void flipMsb() {demulMsb = !demulMsb; digitalWrite(demulMsbPin, !demulMsb);}

    void shiftIn(byte bits) {
        //get the demul bits to 10
        if (demulMsb == false && demulLsb == true) //01
        setLsb(false);
        if (demulLsb == demulMsb) {
        if (demulMsb == false) //00
            setMsb(true);
        else //11
            setLsb(false);
        }
        delayMicroseconds(holdMicros);

        //shift bits in
        for (int i = 7; i >= 0; i--) {
        bool currentBit = (bits >> i) & 1;
        if (currentBit == true) {
            flipLsb();
            delayMicroseconds(holdMicros);
            flipLsb();
        }
        else {
            flipMsb();
            delayMicroseconds(holdMicros);
            flipMsb();
        }
        }
        registerBits = bits;
    }

    void clearRegister(bool latch) {
        pinMode(busPin, INPUT);
        shiftIn(0);
        if (latch) {
        flipMsb();
        delayMicroseconds(holdMicros);
        flipLsb();
        latchBits = registerBits;
        }
    }
}