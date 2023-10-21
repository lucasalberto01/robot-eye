#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>

#include "config.h"
#include "core/state.h"
#include "network/web.h"
#include "persona/persona.h"

short mode = MODE_AP;
TMe me;

// ROM storage
Preferences preferences;

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
    Serial2.begin(9600, SERIAL_8N1, 16, 17);

    // Initialize state
    strcpy(me.ssid, SSID_DEFAULT);
    strcpy(me.password, PASSWORD_DEFAULT);
    me.mode = MODE_AP;
    strcpy(me.hostname, HOST_EXTERNAL);
    me.port = PORT_EXTERNAL;
    strcpy(me.serialNumber, "XX:XX");

    // Get MAC address
    const char* mac = WiFi.macAddress().c_str();

    // Last 5 characters of MAC address to serialNumberS
    for (int i = 0; i < 5; i++) {
        me.serialNumber[i] = mac[i + 9];
    }

    // Initialize EEPROM
    preferences.begin("robot", false);

    // Set WiFi to station mode and disable WiFi sleep mode
    WiFi.setSleep(WIFI_PS_NONE);

    if (!preferences.getBool("configured", false)) {
        preferences.putBool("configured", true);
        preferences.putInt("mode", MODE_AP);
    }

    mode = preferences.getInt("mode", MODE_AP);
    Serial.println("Mode: " + String(mode));

    // CLIENT MODE
    if (mode == MODE_STA) {
        preferences.getBytes("ssid", me.ssid, 32);
        preferences.getBytes("password", me.password, 32);

        Serial.println("Connecting to WiFi network: " + String(me.ssid));

        WiFi.begin(me.ssid, me.password);
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
        }
        Serial.println("Connected");

        // SERVER MODE
    } else if (mode == MODE_AP) {
        // asprintf(&me.ssid, "%s-%s", SSID_DEFAULT, me.serialNumber);

        Serial.println("Serial number: " + String(me.serialNumber));
        Serial.println("Starting AP ssid: " + String(me.ssid));
        // Host a WiFi Access Point
        WiFi.softAP(me.ssid, me.password);
        Serial.print("AP Started IP: ");
        IPAddress IP = WiFi.softAPIP();
        strcpy(me.hostname, strdup(IP.toString().c_str()));
        Serial.println(me.hostname);

    } else {
        Serial.println("Invalid mode");
        return;
    }

    //     // Configures static IP address
    //     if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    //         Serial.println("STA Failed to configure");
    //     }

    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());

    // Send TMe to slave
    Serial2.write((byte*)&me, sizeof(me));

    web.setup(mode);

    persona.begin();

    pinMode(RESER_BTN, INPUT);
}

void loop() {
    persona.runAnimation();

    // if (millis() - state_robot.lastCommandTime >= TIME_TO_SLEEP) {
    //     persona.setState(SLEEPING);
    // } else {
    //     persona.setState(AWAKE);
    // }
}
