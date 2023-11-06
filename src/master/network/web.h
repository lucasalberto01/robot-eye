#ifndef WEB_H
#define WEB_H

#include <ESPAsyncWebServer.h>
#include <WebSocketsClient.h>

#include "../config.h"
#include "../core/state.h"

class Web {
   private:
    /* data */
   public:
    void setup();
    void loop();
    void sendTelemetry(TTelemetry* telemetry);
    static void notFound(AsyncWebServerRequest* request);
    static void onWsEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len);
    static void webSocketEventClient(WStype_t type, uint8_t* payload, size_t length);
    static void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);
    static String indexPageProcessor(const String& var);
};

#endif