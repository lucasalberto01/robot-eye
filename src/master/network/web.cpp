
#include "../network/web.h"

#include <AsyncTCP.h>
#include <ESP32_Servo.h>
#include <WebSocketsClient.h>

#include "../config.h"
#include "../control/control.h"
#include "../light/light.h"
#include "../motor/motor.h"
#include "../persona/persona.h"
#include "../utils.h"

#define MAX_RANGE_HOVERBOARD 1000
#define SPEED_LIMIT 250
#define REVERSE_SPEED_LIMIT -250

WebSocketsClient webSocketsClient;

Persona persona;
Light light;
CamControl camControl;
Servo serDir;
Motor motor;

TTelemetry telemetry;
unsigned long lastCommandTime = 0;

void Web::sendTelemetry() {
    // Get telemetry data
    telemetry.battery = 12.6;
    telemetry.temperature = ((temprature_sens_read() - 32) / 1.8);
    telemetry.signal = WiFi.RSSI();

    // Display telemetry data on the serial monitor
    Serial.printf("[] Telemetry: Battery: %d%%\tSignal: %d dBm\tTemperature: %d°C\n", telemetry.battery, telemetry.signal, telemetry.temperature);

    // Allocate memory for the buffer on the stack
    size_t buffer_size = sizeof(uint8_t) + sizeof(TTelemetry);
    uint8_t buffer[buffer_size];

    // Fill the buffer directly
    buffer[0] = 0;

    // Write the telemetry data directly to the buffer
    *reinterpret_cast<TTelemetry*>(buffer + sizeof(uint8_t)) = telemetry;

    // Send the buffer using webSocketsClient
    webSocketsClient.sendBIN(buffer, buffer_size);
}

void Web::sendCarCommand(char* command) {
    lastCommandTime = millis();

    if (strncmp(command, "motor#", 6) == 0) {
        strtok(strdup(command), "#");
        int dir = atoi(strtok(NULL, "#"));
        int speed = atoi(strtok(NULL, "#"));

        motor.setDirectionCaterpillar(dir, speed);

    } else if (strncmp(command, "servo#", 6) == 0) {
        // Controle da do gimbal da câmera
        strtok(strdup(command), "#");
        int tilt = atoi(strtok(NULL, "#"));
        int pan = atoi(strtok(NULL, "#"));

        Serial.printf("Tilt: %d, Pan: %d\n", tilt, pan);

        camControl.setCamPan(pan);
        camControl.setCamTilt(tilt);
    } else {
        if (strcmp(command, "left") == 0) {
            persona.setAnimation(Persona::E_LOOK_R, true, false, true);
            motor.turnLeft();

        } else if (strcmp(command, "right") == 0) {
            persona.setAnimation(Persona::E_LOOK_L, true, false, true);
            motor.turnRight();

        } else if (strcmp(command, "up") == 0) {
            motor.moveForward(motor.getSpeed());

        } else if (strcmp(command, "down") == 0) {
            motor.moveBackward(motor.getSpeed());
        } else if (strcmp(command, "stop") == 0) {
            motor.stop();

        } else if (strcmp(command, "led") == 0) {
            light.toggle();
        } else if (strcmp(command, "fast") == 0) {
            motor.setCurrentSpeed(speedSettings::FAST);
        } else if (strcmp(command, "normal") == 0) {
            motor.setCurrentSpeed(speedSettings::NORMAL);
        } else if (strcmp(command, "slow") == 0) {
            motor.setCurrentSpeed(speedSettings::SLOW);
        } else {
            persona.processCommand(command);
        }
    }

    return;
}
void Web::webSocketEventClient(WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            handleDisconnected();
            break;

        case WStype_CONNECTED:
            handleConnected();
            break;

        case WStype_TEXT:
            handleTextPayload(payload);
            break;

        case WStype_BIN:
            handleBinaryPayload(length);
            break;

        case WStype_PING:
        case WStype_PONG:
            webSocketsClient.sendPing();
            break;

        case WStype_ERROR:
        case WStype_FRAGMENT_TEXT_START:
        case WStype_FRAGMENT_BIN_START:
        case WStype_FRAGMENT:
        case WStype_FRAGMENT_FIN:
            handleError();
            break;
    }
}

void Web::handleDisconnected() {
    Serial.printf("[] Disconnected!\n");
    motor.stop();
    serDir.write(90);
    camControl.setCenter();
}

void Web::handleConnected() {
    Serial.printf("[] Connected\n");
    camControl.setCenter();
}

void Web::handleTextPayload(uint8_t* payload) {
    Serial.printf("[] Text: %s\n", payload);
    sendCarCommand((char*)payload);
}

void Web::handleBinaryPayload(size_t length) {
    Serial.printf("[] Binary: %u\n", length);
}

void Web::handleError() {
    Serial.printf("[] Error\n");
}

void Web::setup() {
    // Get device ID
    uint32_t id = 0;
    for (int i = 0; i < 17; i = i + 8) {
        id |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }

    String headers = "X-Device-ID: " + String(id) + "\r\nX-Device-Type: ESP32";
    Serial.printf("DeviceID = %d\n", id);

    // Connect to WebSocket server
    webSocketsClient.begin(HOST_ADDR, PORT_ADDR, "/", "ESP32");
    webSocketsClient.setExtraHeaders(headers.c_str());
    webSocketsClient.onEvent(webSocketEventClient);
}

void Web::loop() {
    webSocketsClient.loop();
}
