#ifndef STATE_H
#define STATE_H

#include "../types.h"
#include <stdio.h> 

#define MODE_AP 0   // Access point mode
#define MODE_STA 1  // Station mode

#define MODE_OPERATION MODE_STA



struct TTelemetry_struct {
    int8_t battery;
    int8_t signal;
    int8_t temperature;
};

typedef struct TTelemetry_struct TTelemetry;

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
    unsigned long lastTelemetryTime;
    speedSettings currentSpeed;
};

typedef struct TStateRobot_struct TStateRobot;

#endif  // STATE_H