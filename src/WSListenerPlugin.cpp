/**
 * @file WSListenerPlugin.cpp
 * @author Nghia Ngo Quang (nqnghia285@gmail.com)
 * @brief
 * @version 1.0.1
 * @date 2021-11-05
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "WSListenerPlugin.h"

WSListenerPlugin::WSListenerPlugin() {}

WSListenerPlugin::~WSListenerPlugin() {}

/**
 * @brief Calls a function which matching the event.
 *
 * @param event const char *
 * @param server AsyncWebSocket *
 * @param client AsyncWebSocketClient *
 * @param payload const char *
 */
void WSListenerPlugin::trigger(const char *event, AsyncWebSocket *server, AsyncWebSocketClient *client, const char *payload) {
   auto e = _events.find(event);
   if (e != _events.end()) {
      e->second(server, client, payload);
   } else {
      SOCKETIOCLIENT_DEBUG("[ws] event %s not found. %d events available\n", event, _events.size());
   }
}

/**
 * @brief Gets event name or payload in the msg.
 *
 * @param msg String
 * @param pos ElementOptions
 * @return String
 */
String WSListenerPlugin::getElementInMSG(String msg, ElementOptions pos) {
   const int capacity = FACTOR * msg.length();
   DynamicJsonDocument doc(capacity);

   String result = "";

   // Deserialize msg to array: [event,payload]
   DeserializationError err = deserializeJson(doc, msg);

   if (err == DeserializationError::Ok) {
      // Get a reference to the root array
      JsonArray arr = doc.as<JsonArray>();
      // Get first element that is event name
      result = String((const char *)arr.getElement(pos));

      // Remove character "
      if (result.startsWith("\"")) {
         result.remove(0, 1);
      }

      if (result.endsWith("\"")) {
         result.remove(result.length() - 1);
      }
   }

   // Clear store
   doc.clear();

   // Return result
   return result;
}

/**
 * @brief Handles arrived events.
 *
 * @param server AsyncWebSocket *
 * @param client AsyncWebSocketClient *
 * @param type AwsEventType
 * @param arg void *
 * @param data uint8_t *
 * @param len size_t
 */
void WSListenerPlugin::onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
   if (type == WS_EVT_CONNECT) {
      client->ping();
   } else if (type == WS_EVT_DISCONNECT) {
      SOCKETIOCLIENT_DEBUG("ws[%s][%u] disconnect\n", server->url(), client->id());
   } else if (type == WS_EVT_ERROR) {
      SOCKETIOCLIENT_DEBUG("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t *)arg), (char *)data);
   } else if (type == WS_EVT_PONG) {
      SOCKETIOCLIENT_DEBUG("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char *)data : "");
   } else if (type == WS_EVT_DATA) {
      AwsFrameInfo *info = (AwsFrameInfo *)arg;
      String msg = "";
      if (info->final && info->index == 0 && info->len == len) {
         // the whole message is in a single frame and we got all of it's data
         SOCKETIOCLIENT_DEBUG("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT) ? "text" : "binary", info->len);

         if (info->opcode == WS_TEXT) {
            for (size_t i = 0; i < info->len; i++) {
               msg += (char)data[i];
            }
         } else {
            char buff[3];
            for (size_t i = 0; i < info->len; i++) {
               sprintf(buff, "%02x ", (uint8_t)data[i]);
               msg += buff;
            }
         }
         SOCKETIOCLIENT_DEBUG("%s\n", msg.c_str());
         // Call trigger to handle a arrived event
         trigger(getElementInMSG(msg, EVENT_NAME).c_str(), server, client, getElementInMSG(msg, PAYLOAD).c_str());
      } else {
         // message is comprised of multiple frames or the frame is split into multiple packets
         if (info->index == 0) {
            if (info->num == 0)
               SOCKETIOCLIENT_DEBUG("ws[%s][%u] %s-message start\n", server->url(), client->id(),
                                    (info->message_opcode == WS_TEXT) ? "text" : "binary");
            SOCKETIOCLIENT_DEBUG("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
         }

         SOCKETIOCLIENT_DEBUG("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num,
                              (info->message_opcode == WS_TEXT) ? "text" : "binary", info->index, info->index + len);

         if (info->opcode == WS_TEXT) {
            for (size_t i = 0; i < len; i++) {
               msg += (char)data[i];
            }
         } else {
            char buff[3];
            for (size_t i = 0; i < len; i++) {
               sprintf(buff, "%02x ", (uint8_t)data[i]);
               msg += buff;
            }
         }
         SOCKETIOCLIENT_DEBUG("%s\n", msg.c_str());
         // Call trigger to handle a arrived event
         trigger(getElementInMSG(msg, EVENT_NAME).c_str(), server, client, getElementInMSG(msg, PAYLOAD).c_str());

         if ((info->index + len) == info->len) {
            SOCKETIOCLIENT_DEBUG("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
            if (info->final) {
               SOCKETIOCLIENT_DEBUG("ws[%s][%u] %s-message end\n", server->url(), client->id(),
                                    (info->message_opcode == WS_TEXT) ? "text" : "binary");
            }
         }
      }
   }
}

/**
 * @brief Adds a listener function into _events.
 *
 * @param event const char *
 * @param func std::function<void(AsyncWebSocket *server, AsyncWebSocketClient *client, const char *payload)>
 */
void WSListenerPlugin::on(const char *event, std::function<void(AsyncWebSocket *server, AsyncWebSocketClient *client, const char *payload)> func) {
   _events[event] = func;
}

/**
 * @brief Adds a listener function into _events.
 *
 * @param event String
 * @param func std::function<void(AsyncWebSocket *server, AsyncWebSocketClient *client, const char *payload)>
 */
void WSListenerPlugin::on(String event, std::function<void(AsyncWebSocket *server, AsyncWebSocketClient *client, const char *payload)> func) {
   on(event.c_str(), func);
}

/**
 * @brief Removes the listener function matching the event.
 *
 * @param event const char *
 */
void WSListenerPlugin::remove(const char *event) {
   auto e = _events.find(event);
   if (e != _events.end()) {
      _events.erase(e);
   } else {
      SOCKETIOCLIENT_DEBUG("[ws] event %s not found, can not be removed", event);
   }
}

/**
 * @brief Removes the listener function matching the event.
 *
 * @param event String
 */
void WSListenerPlugin::remove(String event) { remove(event.c_str()); }

/**
 * @brief Removes all listener functions.
 *
 */
void WSListenerPlugin::removeAll() { _events.clear(); }

/**
 * @brief Sends a event to client.
 *
 * @param client AsyncWebSocketClient *
 * @param event const char *
 * @param payload const char *
 */
void WSListenerPlugin::emit(AsyncWebSocketClient *client, const char *event, const char *payload) {
   String msg = "[" + String(event) + "," + String(payload) + "]";
   client->text(msg.c_str());
}

/**
 * @brief Sends a event to client.
 *
 * @param client AsyncWebSocketClient *
 * @param event String
 * @param payload String
 */
void WSListenerPlugin::emit(AsyncWebSocketClient *client, String event, String payload) { emit(client, event.c_str(), payload.c_str()); }

/**
 * @brief Sends a event to all clients.
 *
 * @param server AsyncWebSocket *
 * @param event const char *
 * @param payload const char *
 */
void WSListenerPlugin::emitAll(AsyncWebSocket *server, const char *event, const char *payload) {
   String msg = "[" + String(event) + "," + String(payload) + "]";
   server->textAll(msg.c_str());
}

/**
 * @brief Sends a event to all clients.
 *
 * @param server AsyncWebSocket *
 * @param event String
 * @param payload String
 */
void WSListenerPlugin::emitAll(AsyncWebSocket *server, String event, String payload) { emitAll(server, event.c_str(), payload.c_str()); }