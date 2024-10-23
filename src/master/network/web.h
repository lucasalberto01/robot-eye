
#ifndef WEB_H
#define WEB_H

#include <WebSocketsClient.h>

class Web {
  public:
    void setup();
    void loop();
    void sendTelemetry();
    static void sendCarCommand(char *command);
    static void webSocketEventClient(WStype_t type, uint8_t *payload, size_t length);
    static void handleDisconnected();
    static void handleConnected();
    static void handleTextPayload(uint8_t *payload);
    static void handleBinaryPayload(size_t length);
    static void handleError();
};

#endif
