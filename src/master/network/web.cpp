#include "web.h"

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPI.h>

#include "../core/state.h"
#include "../types.h"
#include "SPIFFS.h"

// AsyncWebserver runs on port 80 and the asyncwebsocket is initialize at this point also
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

extern TStateRobot state_robot;

// Processor for index.html page template.  This sets the radio button to checked or unchecked
String Web::indexPageProcessor(const String& var) {
    String status = "";
    if (var == "SPEED_SLOW_STATUS") {
        if (state_robot.currentSpeed == speedSettings::SLOW) {
            status = "checked";
        }
    } else if (var == "SPEED_NORMAL_STATUS") {
        if (state_robot.currentSpeed == speedSettings::NORMAL) {
            status = "checked";
        }
    } else if (var == "SPEED_FAST_STATUS") {
        if (state_robot.currentSpeed == speedSettings::FAST) {
            status = "checked";
        }
    }
    return status;
}

// Callback function that receives messages from websocket client
void Web::onWsEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
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
                    // sendCarCommand(command);
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
void Web::notFound(AsyncWebServerRequest* request) {
    request->send(404, "text/plain", "Not found");
}

void Web::setup() {
    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }

    // Add callback function to websocket server
    ws.onEvent([](AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
        // Web::onWsEvent(server, client, type, arg, data, len);
    });

    server.addHandler(&ws);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        Serial.println("Requesting index page...");
        request->send(SPIFFS, "/index.html", "text/html", false, indexPageProcessor);
    });

    // Route to load bootstrap.min.css file
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
