#include <LittleFS.h>
#include <WSListenerPlugin.h>

#ifdef ESP32
#include <ESPmDNS.h>

#elif defined(ESP8266)
#include <ESP8266mDNS.h>
#endif

#define WIFI_CONFIG_FILE_PATH "/wifi_config.txt"

#define CSM "client-send-message"

#define SSM "server-send-message"

// SKETCH BEGIN
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
WSListenerPlugin wslp;

// Params
const char *hostName = "nodemcu-12e";
const char *password = "admin123";
const char *domain = "nodemcuv2";

// void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
//    wslp.onEvent(server, client, type, arg, data, len);
// }

String readTextFile(const char *filePath) {
   String data = String();
   File file = LittleFS.open(filePath, "r");
   if (!file || file.isDirectory()) {
      return "";
   }
   while (file.available()) {
      data += char(file.read());
   }
   file.close();

   return data;
}

////////////////////////////////////////////////
void clientSendMessage(AsyncWebSocket *server, AsyncWebSocketClient *client, const char *payload) {
   Serial.printf("[%s]: %s", CSM, payload);

   wslp.emit(client, SSM, "Hi! Client!");
}
////////////////////////////////////////////////

void configServer() {
   if (MDNS.begin(domain)) { // local domain name: ${domain}.local
      Serial.println("MDNS responder started!");
      Serial.printf("Local Domain Name: %s", domain);
      MDNS.addService("_http", "_tcp", 23);
   }

   server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(200, "text/plain", String(ESP.getFreeHeap())); });

   // Route for root / web page
   server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

   // Handle not found pages
   server.onNotFound([](AsyncWebServerRequest *request) {
      Serial.printf("NOT_FOUND: ");
      if (request->method() == HTTP_GET)
         Serial.printf("GET");
      else if (request->method() == HTTP_POST)
         Serial.printf("POST");
      else if (request->method() == HTTP_DELETE)
         Serial.printf("DELETE");
      else if (request->method() == HTTP_PUT)
         Serial.printf("PUT");
      else if (request->method() == HTTP_PATCH)
         Serial.printf("PATCH");
      else if (request->method() == HTTP_HEAD)
         Serial.printf("HEAD");
      else if (request->method() == HTTP_OPTIONS)
         Serial.printf("OPTIONS");
      else
         Serial.printf("UNKNOWN");
      Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

      if (request->contentLength()) {
         Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
         Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
      }

      int headers = request->headers();
      int i;
      for (i = 0; i < headers; i++) {
         AsyncWebHeader *h = request->getHeader(i);
         Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
      }

      int params = request->params();
      for (i = 0; i < params; i++) {
         AsyncWebParameter *p = request->getParam(i);
         if (p->isFile()) {
            Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
         } else if (p->isPost()) {
            Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
         } else {
            Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
         }
      }

      request->send(404, "text/html",
                    "<!doctype html><html lang=\"en\"><head><meta charset=\"utf-8\"/><link rel=\"icon\" href=\"/favicon.ico\"/><meta "
                    "name=\"viewport\" content=\"width=device-width,initial-scale=1\"/><meta name=\"theme-color\" content=\"#000000\"/><meta "
                    "name=\"description\" content=\"Web site created using create-react-app\"/><title>React "
                    "App</title></head><body><h1 style=\"margin:0 auto;\">Not "
                    "Found!</h1><script>setTimeout(()=>{alert(\"NotFound!\")},3000)</script></body></html>");
   });

   // Get body
   server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      if (!index)
         Serial.printf("BodyStart: %u\n", total);
      Serial.printf("%s", (const char *)data);
      if (index + len == total)
         Serial.printf("BodyEnd: %u\n", total);
   });

   // Adds listener functions
   wslp.on(CSM, clientSendMessage);

   // Begin server
   ws.onEvent(std::bind(&wslp.onEvent, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4,
                        std::placeholders::_5, std::placeholders::_6));
   server.addHandler(&ws);

   server.begin();
}

void configWiFi() {
   WiFi.mode(WIFI_AP_STA);
   WiFi.softAP(hostName, password);

   if (!LittleFS.begin()) {
      Serial.println("An Error has occurred while mounting LittleFS");
      return;
   }

   // Get wifi config
   String input = readTextFile(WIFI_CONFIG_FILE_PATH);
   Serial.printf("\r\nData: %s\r\n", input.c_str());

   const int capacity = FACTOR * JSON_OBJECT_SIZE(2);
   DynamicJsonDocument doc(capacity);
   DeserializationError err = deserializeJson(doc, input);
   if (err) {
      Serial.print(F("deserializeJson() failed with code "));
      Serial.println(err.f_str());
      return;
   }

   JsonObject wifi = doc.as<JsonObject>();

   // Connect to wifi
   if (wifi.containsKey("ssid") && wifi.containsKey("pwd")) {
      const char *ssid = wifi["ssid"].as<const char *>();
      const char *pwd = wifi["pwd"].as<const char *>();

      Serial.printf("Connecting to %s", ssid);
      WiFi.begin(ssid, pwd);

      //  Wait esp8266 connected to wifi
      while (WiFi.status() != WL_CONNECTED) {
         delay(500);
         Serial.print('.');
      }

      Serial.printf("\r\nConnected WiFi: %s\r\n", ssid);
      Serial.printf("IP address(ESP): %s\r\n", WiFi.localIP().toString().c_str());
   }

   doc.clear();
}

void setup() {
   // put your setup code here, to run once:
   Serial.begin(921600);
   Serial.setDebugOutput(true);

   // Config WiFi
   configWiFi();

   // Config web server
   configServer();
}

void loop() {
   // put your main code here, to run repeatedly:
   ws.cleanupClients();
}