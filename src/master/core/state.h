#ifndef STATE_H
#define STATE_H

#include "../types.h"

struct TStateRobot_struct {
    unsigned long lastCommandTime;
    speedSettings currentSpeed;
};

typedef struct TStateRobot_struct TStateRobot;

#endif  // STATE_H