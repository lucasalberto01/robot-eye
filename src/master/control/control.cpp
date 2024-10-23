#include "control.h"

#include <Arduino.h>
#include <ESP32_Servo.h>

#include "../config.h"

Servo tiltControl;
Servo panControl;

CamControl::CamControl() {
}

void CamControl::setup() {
    tiltControl.attach(SERVO_CAM_TITL);
    panControl.attach(SERVO_CAM_PAN);
    setCenter();
}

void CamControl::setCamTilt(int angle) {
    tiltControl.write(checkLimit(angle));
}

void CamControl::setCamPan(int angle) {
    panControl.write(checkLimit(angle));
}

void CamControl::setCenter() {
    tiltControl.write(90);
    panControl.write(90);
}

int CamControl::checkLimit(int angle) {
    if (angle > 180) {
        return 180;
    } else if (angle < 0) {
        return 0;
    } else {
        return angle;
    }
}

void CamControl::test() {
    for (int i = 0; i < 180; i++) {
        setCamTilt(i);
        setCamPan(i);
        delay(10);
    }
}
