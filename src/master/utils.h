#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

enum speedSettings {
    SLOW = 165,
    NORMAL = 185,
    FAST = 255
};

struct TTelemetry_struct {
    int8_t battery;
    int8_t signal;
    int8_t temperature;
};

typedef struct TTelemetry_struct TTelemetry;

typedef struct TMe_struct TMe;

struct TStateRobot_struct {
    unsigned long lastCommandTime;
    unsigned long lastTelemetryTime;
    speedSettings currentSpeed;
};

typedef struct TStateRobot_struct TStateRobot;

#ifdef __cplusplus
extern "C" {
#endif

uint8_t temprature_sens_read();

#ifdef __cplusplus
}
#endif

uint8_t temprature_sens_read();

#endif  // UTILS_H
