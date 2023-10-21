#include "web.h"

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPI.h>
#include <WebSocketsClient.h>
#include <WebSocketsServer.h>

#include "../core/core.h"
#include "../core/state.h"
#include "../types.h"
#include "SPIFFS.h"

// AsyncWebserver runs on port 80 and the asyncwebsocket is initialize at this point also
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
WebSocketsClient webSocketsClient;

uint8_t* buffer = NULL;

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
#if (MODE_OPERATION == MODE_AP)  // SERVER MODE
    switch (type) {
        case WS_EVT_CONNECT: {
            AsyncWebServerRequest* request = (AsyncWebServerRequest*)arg;

            Serial.println("Header values:");
            int headers = request->headers();
            bool isCam = false;
            for (int i = 0; i < headers; i++) {
                AsyncWebHeader* h = request->getHeader(i);
                if (h->name() == "Sec-WebSocket-Protocol" && h->value() == "ESPCAM") {
                    Serial.println("Found ESPCAM header");
                    clients[0] = client;
                    client->printf("start");
                    isCam = true;
                }
            }

            if (!isCam) {
                Serial.println("Found ESP32 header");
                if (clients[1] != NULL && clients[1]->id() > 0) {
                    ws.close(clients[1]->id());
                }
                clients[1] = client;
            }

            Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
            break;
        }

        case WS_EVT_DISCONNECT: {
            Serial.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
            for (int i = 0; i < 2; i++) {
                if (clients[i] != NULL && clients[i]->id() == client->id()) {
                    clients[i] = NULL;
                }
            }
            break;
        }

        case WS_EVT_DATA: {
            // data packet
            AwsFrameInfo* info = (AwsFrameInfo*)arg;
            if (len > 0 && info->final && info->index == 0 && info->len == len) {
                // the whole message is in a single frame and we got all of it's data
                Serial.println("Single frame message");
                if (info->opcode == WS_TEXT) {
                    data[len] = 0;
                    char* command = (char*)data;
                    sendCarCommand(command);
                }
            } else {
                // message is comprised of multiple frames or the frame is split into multiple packets

                if ((info->index + len) == info->len) {
                    if (info->final) {
                        if (clients[1] != NULL && clients[1]->id() > 0) {
                            ws.binary(clients[1]->id(), data, info->len);
                        }

                        Serial.printf("command[%u]:\n", info->len);
                    }
                }
                // message is comprised of multiple frames or the frame is split into multiple packets
            }
            break;
        }

        case WS_EVT_PONG: {
            Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char*)data : "");
            break;
        }

        case WS_EVT_ERROR: {
            Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
            break;
        }
    }
#endif
}

// Function called when resource is not found on the server
void Web::notFound(AsyncWebServerRequest* request) {
    request->send(404, "text/plain", "Not found");
}

void Web::webSocketEventClient(WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            Serial.printf("[] Disconnected!\n");

            break;
        case WStype_CONNECTED:
            Serial.printf("[] Connected\n");

            break;
        case WStype_TEXT:
            Serial.printf("[] Text: %s\n", payload);
            sendCarCommand((char*)payload);

            break;
        case WStype_BIN:
            Serial.printf("[] Binary: %u\n", length);
            break;
        case WStype_ERROR:
        case WStype_FRAGMENT_TEXT_START:
        case WStype_FRAGMENT_BIN_START:
        case WStype_FRAGMENT:
        case WStype_FRAGMENT_FIN:
            Serial.printf("[] Error\n");
            break;
    }
}

void Web::setup() {
    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }

#if (MODE_OPERATION == MODE_AP)  // SERVER MODE

    // Add callback function to websocket server
    ws.onEvent(onWsEvent);

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

#endif
#if (MODE_OPERATION == MODE_STA)  // CLIENT MODE

    // Iniciação do WebSocket
    webSocketsClient.begin(HOST_ADDR, PORT_ADDR, "/", "ESP32");
    webSocketsClient.setExtraHeaders("X-Auth-Token: 223\r\nX-Device-ID: 223\r\nX-Device-Type: ESP32\r\n");
    webSocketsClient.onEvent(webSocketEventClient);
#endif
}

void Web::loop() {
#if (MODE_OPERATION == MODE_STA)  // CLIENT MODE
    webSocketsClient.loop();
#endif
}
