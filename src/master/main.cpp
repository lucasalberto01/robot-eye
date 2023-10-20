#include <Arduino.h>
#include <WiFi.h>

#include "config.h"
#include "core/state.h"
#include "network/web.h"
#include "persona/persona.h"

// Change this to your network SSID
char ssid[] = "ROBOT-xxxxx";
char password[] = "robo1234";

#define ENABLE_CLIENT 0  // Set to 1 to enable client mode

#if ENABLE_CLIENT
// Set your Static IP address
IPAddress local_IP(192, 168, 1, 1);
// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);    // optional
IPAddress secondaryDNS(8, 8, 4, 4);  // optional
#endif

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
    Serial.println("Connecting to ");
    Serial.println(ssid);
    WiFi.setSleep(WIFI_PS_NONE);

#if ENABLE_CLIENT
    // Configures static IP address
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
        Serial.println("STA Failed to configure");
    }

    // Connect to WiFi network
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
#else
    const char* mac = WiFi.macAddress().c_str();  // Get MAC address
    for (uint8_t i = 6; i < 11; i++) {            // Update SSID with mac address
        ssid[i] = mac[i + 6];
    }
    // Host a WiFi Access Point
    WiFi.softAP(ssid, password);
#endif

    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());

    web.setup();

    persona.begin();
}

void loop() {
    persona.runAnimation();

    if (millis() - state_robot.lastCommandTime >= TIME_TO_SLEEP) {
        persona.setState(SLEEPING);
    } else {
        persona.setState(AWAKE);
    }
}
