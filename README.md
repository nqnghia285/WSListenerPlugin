# WSListenerPlugin [![Build Status](https://github.com/Links2004/arduinoWebSockets/workflows/CI/badge.svg?branch=master)](https://github.com/nqnghia285/WSListenerPlugin)

A plugin supports configuring listeners in web server used websocket: [AsyncWebSocket](https://github.com/me-no-dev/ESPAsyncWebServer).

##### Supported Hardware

-  ESP8266 [Arduino for ESP8266](https://github.com/esp8266/Arduino/)
-  ESP32 [Arduino for ESP32](https://github.com/espressif/arduino-esp32)

### Dependencies:

-  ESPAsyncWebServer library (v1.2.3).
-  ArduinoJson library (v6.18.5).

### Message format

```c++
   Message: ["event_name","payload"]
```

#### Example:

```c++
   // A string is sent from client(websocket in browser)
   String msg = "[\"control\",\"{\"led-01\":\"on\",\"led-02\":\"off\"}\"]";
```

```c++
   // Sends a event
   WSListenerPlugin wslp;

   ...// config something

   void clientSendMessage(AsyncWebSocket *server, AsyncWebSocketClient *client, const char *payload) {
      String eventName = "message";
      String message = "Hello client!";

      // Sends a event to a client
      wslp.emit(client, eventName, message);

      // Sends a event to all clients
   wslp.emitAll(server, eventName, message);
   }
```

### High Level Client API

-  `onEvent`: Handles arrived events.

```c++
    void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
```

-  `on` : Adds a listener function into \_events.

```c++
    void on(const char *event, std::function<void(AsyncWebSocket *server, AsyncWebSocketClient *client, const char *payload)> func);
```

```c++
    void on(String event, std::function<void(AsyncWebSocket *server, AsyncWebSocketClient *client, const char *payload)> func);
```

-  `remove` : Removes the listener function matching the event.

```c++
    void remove(const char *event);
```

```c++
    void remove(String event);
```

-  `removeAll` : Removes all listener functions.

```c++
    void removeAll(void);
```

-  `emit` : Sends a event to client.

```c++
    void emit(AsyncWebSocketClient *client, const char *event, const char *payload);
```

```c++
    void emit(AsyncWebSocketClient *client, String event, String payload);
```

-  `emitAll` : Sends a event to all clients.

```c++
    void emitAll(AsyncWebSocket *server, const char *event, const char *payload);
```

```c++
    void emitAll(AsyncWebSocket *server, String event, String payload);
```

### Example

Visit to [here](https://github.com/nqnghia285/WSListenerPlugin/blob/master/examples/WebServerWithWSListenerPlugin.cpp)

### Issues

Submit issues to [here](https://github.com/nqnghia285/WSListenerPlugin/issues)
