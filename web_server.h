#ifndef WEB_SERVER_H
#define WEB_ SERVER_H

#include <ESPAsyncWebServer.h>
#include "config.h"

// Declare global instances of AsyncWebServer and AsyncWebSocket
extern AsyncWebServer server;
extern AsyncWebSocket ws;

// Initialize Async Web Server, WebSocket handlers, and endpoint routes
void webServerBegin();

// Broadcast real-time radar data payload to all connected WebSockets
void webServerBroadcastRadar(int angle, long dist, long frontDist, String alert);

#endif // WEB_SERVER_H
