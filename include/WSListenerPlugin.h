/**
 * @file WSListenerPlugin.h
 * @author Nghia Ngo Quang (nqnghia285@gmail.com)
 * @brief
 * @version 1.0.1
 * @date 2021-11-05
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef WSLISTENERPLUGIN_H_
#define WSLISTENERPLUGIN_H_

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <map>

#ifdef ESP32
#include <AsyncTCP.h>
#include <WiFi.h>

#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif

#define SOCKETIOCLIENT_DEBUG(...)

#define FACTOR 4

typedef enum {
   EVENT_NAME = 0,
   PAYLOAD = 1,
} ElementOptions;

class WSListenerPlugin {
 protected:
   std::map<String, std::function<void(AsyncWebSocket *server, AsyncWebSocketClient *client, const char *payload)>> _events;

   void trigger(const char *event, AsyncWebSocket *server, AsyncWebSocketClient *client, const char *payload);
   String getElementInMSG(String msg, ElementOptions pos);

 public:
   WSListenerPlugin(void);
   virtual ~WSListenerPlugin(void);

   void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
   void on(const char *event, std::function<void(AsyncWebSocket *server, AsyncWebSocketClient *client, const char *payload)>);
   void on(String event, std::function<void(AsyncWebSocket *server, AsyncWebSocketClient *client, const char *payload)>);
   void remove(const char *event);
   void remove(String event);
   void removeAll(void);
   void emit(AsyncWebSocketClient *client, const char *event, const char *payload);
   void emit(AsyncWebSocketClient *client, String event, String payload);
   void emitAll(AsyncWebSocket *server, const char *event, const char *payload);
   void emitAll(AsyncWebSocket *server, String event, String payload);
}

#endif /* WSLISTENERPLUGIN_H_ */