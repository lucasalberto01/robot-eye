#ifndef STATE_H
#define STATE_H

#include "../types.h"

#define MODE_AP 0
#define MODE_STA 1

struct TMe_struct {
    char ssid[48];
    char password[48];
    short mode;
    char hostname[48];
    char serialNumber[6];
    int port;
};

typedef struct TMe_struct TMe;

struct TStateRobot_struct {
    unsigned long lastCommandTime;
    speedSettings currentSpeed;
};

typedef struct TStateRobot_struct TStateRobot;

#endif  // STATE_H