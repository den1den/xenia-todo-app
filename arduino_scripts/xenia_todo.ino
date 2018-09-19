#include <Arduino.h>
#include <ESP8266WiFi.h> # http://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/client-examples.html
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h> # https://arduinojson.org/
#include <SPI.h>

//SSID of your network
const char WIFI_SSID[] = "dennis"; //SSID of your Wi-Fi router
const char WIFI_PASS[] = "sillybutter708"; //Password of your Wi-Fi router
const int WEMOS_ID = 1;

const char API_SERVER[] = "192.168.1.50"; //Address of the server
const int API_PORT = 7777;
const char API_ADDRESS[] = "/api/wemos-todos/all";

void setup()
{
  Serial.begin(9600);
  Serial.println("");
  while (!Serial) {}
  Serial.println("Serial connected");
}

// Use arduinojson.org/assistant to compute the capacity.
const size_t capacity = JSON_ARRAY_SIZE(30) + 30 * JSON_OBJECT_SIZE(7) + 245;
DynamicJsonBuffer jsonBuffer(capacity);

void connectToServer(){
  Serial.printf("Connecting to %s", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {delay(100);}
  Serial.println(", connected");
}

void loop() {
  WiFiClient client;

  if(!client.connect(API_SERVER, API_PORT))
  {
    return;
  }
  Serial.printf("Connected to %s\n", API_SERVER);
  client.print(
    String("GET ") + API_ADDRESS + "?wemos=" + String(WEMOS_ID) + " HTTP/1.1\r\n" +
    "Host: " + API_SERVER + "\r\n" +
    "Connection: close\r\n" +
    "\r\n"
  );

  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
    Serial.print("Unexpected response: ");
    Serial.println(status);
    delay(10000);
    return;
  }
  
  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println("Invalid response");
    delay(10000);
    return;
  }
  
  // Parse JSON object
  jsonBuffer.clear();
  JsonArray& todos = jsonBuffer.parseArray(client);
  if (!todos.success()) {
    Serial.println(F("Parsing failed!"));
    delay(10000);
    return;
  }
  Serial.println("JSON received");
  client.stop();

  Serial.print("Todo[0] = ");
  JsonObject& firstTodo = todos.get<JsonObject>(0);
  int firstId = firstTodo["id"];
  const char* firstName = firstTodo["name"];
  Serial.printf("%d: %s", firstId, firstName);
  Serial.println("");
  
  delay(5000);
}

