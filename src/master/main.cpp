#include <Arduino.h>
#include <WiFi.h>

#include "config.h"
#include "control/control.h"
#include "motor/motor.h"
#include "network/web.h"
#include "persona/persona.h"

#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();

// Interface to human
Web web;

// Idle animation
extern Persona persona;

// State
extern TStateRobot state_robot;

// Telemetry
extern TTelemetry telemetry;

// Cam control
extern CamControl camControl;

// Motor
extern Motor motor;

unsigned long iTimeTelemetry = 0;
extern unsigned long lastCommandTime;

// Setup function
void setup() {
    // Initialize the serial monitor baud rate
    Serial.begin(115200);
    Serial.setDebugOutput(true);

    // Set WiFi to station mode and disable WiFi sleep mode
    WiFi.setSleep(WIFI_PS_NONE);

    // Setup all the components
    web.setup();
    persona.begin();
    camControl.setup();
    motor.setup();

    // Test all the components
    camControl.test();
    camControl.setCenter();

    // Initialize communication
    Serial.println("Connecting to WiFi network: " + String(WIFI_SSID));
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    uint32_t notConnectedCounter = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.println("Wifi connecting...");
        notConnectedCounter++;
        // Reset board if not connected after 5s
        if (notConnectedCounter > 50) {
            Serial.println("Resetting due to Wifi not connecting...");
            ESP.restart();
        }
    }

    Serial.println("Connected");
    Serial.println("Setup done");
}

void loop() {
    web.loop();
    persona.runAnimation();

    if (millis() - iTimeTelemetry >= 1000) {
        web.sendTelemetry();
        iTimeTelemetry = millis();
    }

    // persona loop check
    if (millis() - lastCommandTime >= TIME_TO_SLEEP) {
        persona.setState(SLEEPING);
    } else {
        persona.setState(AWAKE);
    }
}
