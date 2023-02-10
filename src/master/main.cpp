#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPI.h>
#include <WiFi.h>

#include "SPIFFS.h"
#include "car/car.h"
#include "personality/RobotEyes.h"
#include "personality/voice.h"

// Change this to your network SSID
const char* ssid = "ROBO";
const char* password = "robo1234";

// AsyncWebserver runs on port 80 and the asyncwebsocket is initialize at this point also
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW  ///< Use FC-16 style hardware module.

#define MAX_DEVICES 2
#define CS_PIN 5

MD_MAX72XX M = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

#define LED_PIN 13

// Our car object
Car car;
MD_RobotEyes eyes;
Voice voice;

unsigned long lastCommandTime = 0;
unsigned long timeToSleep = 15000;
bool ledLightOn = false;

void ledToggle() {
    ledLightOn = !ledLightOn;
    digitalWrite(LED_PIN, ledLightOn);
}

// Function to send commands to car
void sendCarCommand(const char* command) {
    // command could be either "left", "right", "forward" or "reverse" or "stop"
    // or speed settings "slow-speed", "normal-speed", or "fast-speed"
    lastCommandTime = millis();

    // Advance movement
    if (strstr(command, "motor#") != NULL) {
        strtok(strdup(command), "#");
        int angle = atoi(strtok(NULL, "#"));
        int speed = atoi(strtok(NULL, "#"));

        // Send command to car
        if ((angle > 80) && (angle < 100)) {
            car.moveForward(speed);
        } else if ((angle > 260) && (angle < 280)) {
            car.moveBackward(speed);
        } else if ((angle > 170) && (angle < 190)) {
            car.turn(0, speed);
        } else if ((angle > 350) || (angle < 10)) {
            car.turn(speed, 0);
        } else if ((angle > 100) && (angle < 170)) {
            car.turn(speed * 7 / 10, speed);
        } else if ((angle > 10) && (angle < 80)) {
            car.turn(speed, speed * 7 / 10);
        } else if ((angle > 190) && (angle < 260)) {
            car.turn(-1 * speed * 7 / 10, -1 * speed);
        } else if ((angle > 280) && (angle < 350)) {
            car.turn(-1 * speed, -1 * speed * 7 / 10);
        } else {
            car.stop();
        }

    }
    // Basic movement
    else if (strcmp(command, "left") == 0) {
        eyes.setAnimation(MD_RobotEyes::E_LOOK_R, true, false, true);
        car.turnLeft();
    } else if (strcmp(command, "right") == 0) {
        eyes.setAnimation(MD_RobotEyes::E_LOOK_L, true, false, true);
        car.turnRight();
    } else if (strcmp(command, "up") == 0) {
        car.moveForward(car.getSpeed());
    } else if (strcmp(command, "down") == 0) {
        car.moveBackward(car.getSpeed());
    } else if (strcmp(command, "stop") == 0) {
        car.stop();
    } else if (strcmp(command, "slow-speed") == 0) {
        car.setCurrentSpeed(speedSettings::SLOW);
    } else if (strcmp(command, "normal-speed") == 0) {
        car.setCurrentSpeed(speedSettings::NORMAL);
    } else if (strcmp(command, "fast-speed") == 0) {
        car.setCurrentSpeed(speedSettings::FAST);
    }
    // Illumination
    else if (strcmp(command, "led") == 0) {
        ledToggle();
    }
    // Emotions
    else if (strcmp(command, "neutral") == 0) {
        eyes.setAnimation(MD_RobotEyes::E_NEUTRAL, true, false, true);
    } else if (strcmp(command, "blink") == 0) {
        eyes.setAnimation(MD_RobotEyes::E_BLINK, true, false, true);
    } else if (strcmp(command, "angry") == 0) {
        eyes.setAnimation(MD_RobotEyes::E_ANGRY, true, false, true);
    } else if (strcmp(command, "sad") == 0) {
        eyes.setAnimation(MD_RobotEyes::E_SAD, true, false, true);
    } else if (strcmp(command, "evil") == 0) {
        eyes.setAnimation(MD_RobotEyes::E_EVIL, true, false, true);
    } else if (strcmp(command, "evil2") == 0) {
        eyes.setAnimation(MD_RobotEyes::E_EVIL2, true, false, true);
    } else if (strcmp(command, "squint") == 0) {
        eyes.setAnimation(MD_RobotEyes::E_SQUINT, true, false, true);
    } else if (strcmp(command, "dead") == 0) {
        eyes.setAnimation(MD_RobotEyes::E_DEAD, true, false, true);
    } else {
        eyes.setText(strdup(command));
    }
}

// Processor for index.html page template.  This sets the radio button to checked or unchecked
String indexPageProcessor(const String& var) {
    String status = "";
    if (var == "SPEED_SLOW_STATUS") {
        if (car.getCurrentSpeed() == speedSettings::SLOW) {
            status = "checked";
        }
    } else if (var == "SPEED_NORMAL_STATUS") {
        if (car.getCurrentSpeed() == speedSettings::NORMAL) {
            status = "checked";
        }
    } else if (var == "SPEED_FAST_STATUS") {
        if (car.getCurrentSpeed() == speedSettings::FAST) {
            status = "checked";
        }
    }
    return status;
}

// Callback function that receives messages from websocket client
void onWsEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT: {
            Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
            // client->printf("Hello Client %u :)", client->id());
            // client->ping();
        }

        case WS_EVT_DISCONNECT: {
            Serial.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
        }

        case WS_EVT_DATA: {
            // data packet
            AwsFrameInfo* info = (AwsFrameInfo*)arg;
            if (len > 0 && info->final && info->index == 0 && info->len == len) {
                // the whole message is in a single frame and we got all of it's data
                if (info->opcode == WS_TEXT) {
                    data[len] = 0;
                    char* command = (char*)data;
                    sendCarCommand(command);
                }
            }
        }

        case WS_EVT_PONG: {
            Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char*)data : "");
        }

        case WS_EVT_ERROR: {
            // Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t *)arg), (char *)data);
        }
    }
}

// Function called when resource is not found on the server
void notFound(AsyncWebServerRequest* request) {
    request->send(404, "text/plain", "Not found");
}

void setupDisplay() {
    M.begin();
    eyes.begin(&M);
}

// Setup function
void setup() {
    // Initialize the serial monitor baud rate
    Serial.begin(115200);
    Serial.println("Connecting to ");
    Serial.println(ssid);

    // Connect to WiFi network
    WiFi.softAP(ssid, password);

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }

    setupDisplay();
    pinMode(LED_PIN, OUTPUT);

    // Add callback function to websocket server
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        Serial.println("Requesting index page...");
        request->send(SPIFFS, "/index.html", "text/html", false, indexPageProcessor);
    });

    // Route to load entireframework.min.css file
    server.on("/css/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(SPIFFS, "/css/bootstrap.min.css", "text/css");
    });

    // Route to load custom.css file
    server.on("/css/custom.css", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(SPIFFS, "/css/custom.css", "text/css");
    });

    // Route to load custom.js file
    server.on("/js/custom.js", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(SPIFFS, "/js/custom.js", "text/javascript");
    });

    // On Not Found
    server.onNotFound(notFound);

    // Start server
    server.begin();
}

void loop() {
    eyes.runAnimation();

    if (millis() - lastCommandTime >= timeToSleep) {
        eyes.setState(SLEEPING);
    } else {
        eyes.setState(AWAKE);
    }
}
