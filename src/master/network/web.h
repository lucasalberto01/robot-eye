#ifndef WEB_H
#define WEB_H

#include <ESPAsyncWebServer.h>

#include "../config.h"

class Web {
   private:
    /* data */
   public:
    void setup();
    static void notFound(AsyncWebServerRequest* request);
    void onWsEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len);
    static String indexPageProcessor(const String& var);
};

#endif