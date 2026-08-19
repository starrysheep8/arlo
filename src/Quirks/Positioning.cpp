#include "../Arlo.h"

namespace Arlo {
    const int scanTries = 3;
    const int initialScanTries = 5;

    const int trackedAngleCount = 360;
    uint16_t scanBuffer[trackedAngleCount];
    uint16_t space[trackedAngleCount];

    //ahhhhhhh if something is varrying on the initial scan, marj it and come back to it later. If it continues to vary in the same way, it's part of the room, otherwise it is left blank

    float establishSpace() { //overwrites and initializes the space

    }
    
    float getSpaceIntegrity() { //how much of the space is filled out
    
    }
    
    vec2_t getSpaceCenter() {
    
    }

}