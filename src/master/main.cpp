#include <Arduino.h>
#include <WiFi.h>

#include "config.h"
#include "core/state.h"
#include "network/web.h"
#include "persona/persona.h"

// Interface to human
Web web;

// Idle animation
extern Persona persona;

// State
extern TStateRobot state_robot;

// Setup function
void setup() {
    // Initialize the serial monitor baud rate
    Serial.begin(115200);
    Serial.setDebugOutput(true);

    // Set WiFi to station mode and disable WiFi sleep mode
    WiFi.setSleep(WIFI_PS_NONE);
    Serial.println("Robo mode: " + (MODE_OPERATION == MODE_STA ? String("STA") : String("AP")));

#if (MODE_OPERATION == MODE_STA)  // CLIENT MODE

    Serial.println("Connecting to WiFi network: " + String(WIFI_SSID));

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected");

#endif
#if (MODE_OPERATION == MODE_AP)                   // SERVER MODE
    char ssid[] = "ROBOT-xxxxx";                  // SSID of the ESP32 in AP mode
    const char* mac = WiFi.macAddress().c_str();  // Get MAC address

    for (uint8_t i = 6; i < 11; i++) {  // Update SSID with mac address
        ssid[i] = mac[i + 6];
    }

    //     // Configures static IP address
    //     if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    //         Serial.println("STA Failed to configure");
    //     }

    // Host a WiFi Access Point
    WiFi.softAP(ssid, PASSWORD_DEFAULT);

#endif

    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());

    web.setup();
    persona.begin();

    pinMode(RESER_BTN, INPUT);
}

void loop() {
    web.loop();
    persona.runAnimation();

    if (millis() - state_robot.lastCommandTime >= TIME_TO_SLEEP) {
        persona.setState(SLEEPING);
    } else {
        persona.setState(AWAKE);
    }
}
