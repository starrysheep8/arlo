#include "Arlo.h"

namespace Arlo {
    // when switching between arm and body movement, there needs to be a small delay or else the servo set first gets confused
    Servo headServo;
    Servo armUpperServo;
    Servo busServo;

    Motion headMotion(headServo, 90.0f, 0.0f, Motion::ROBOTIC);
    Motion armUpperMotion(armUpperServo, 0.0f, 0.0f, Motion::ROBOTIC);
    Motion armLowerMotion(busServo, 0.0f, 0.0f, Motion::ROBOTIC);
    Motion bodyMotion(busServo, 0.0f, 0.0f, Motion::ROBOTIC);

    void motionHandler() {

    }

}