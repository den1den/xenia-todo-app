#include <Arduino.h>
#include <ESP8266WiFi.h> # http://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/client-examples.html
#include <ESP8266HTTPClient.h>
#include <TFT_22_ILI9225.h>
#include <ArduinoJson.h> # https://arduinojson.org/
#include <SPI.h>

//SSID of your network
const char WIFI_SSID[] = "dennis"; //SSID of your Wi-Fi router
const char WIFI_PASS[] = "sillybutter708"; //Password of your Wi-Fi router
const int WEMOS_ID = 1;

const char API_SERVER[] = "192.168.1.50"; //Address of the server
const int API_PORT = 7777;
const char API_ADDRESS[] = "/api/wemos-todos/all";

#define NO_PIN 0

#define ROTERY_ENC_A D4
#define ROTERY_ENC_B D1
#define ROTERY_ENC_BUTTON D2

#define TFT_RST D8 // of the screen on RST of Wemos
#define TFT_RS  D6
#define TFT_CS  NO_PIN  // SS
#define TFT_SDI D7 // MOSI
#define TFT_CLK D5 // SCK
#define TFT_LED NO_PIN
#define TFT_BRIGHTNESS 200 // Initial brightness of TFT backlight (optional)

#define MOTION_DETECTOR_DATA_PIN D3
#define WAKEUP_PIN D0

volatile int next_rotery_counter = 0;
volatile int prev_rotery_counter = 0;
volatile int next_press_counter = 0;
volatile int prev_press_counter = 0;
TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_SDI, TFT_CLK, TFT_LED, TFT_BRIGHTNESS);

void setup()
{
  pinMode(WAKEUP_PIN, OUTPUT);
  digitalWrite(WAKEUP_PIN, LOW);

  pinMode(MOTION_DETECTOR_DATA_PIN, INPUT);

  pinMode(ROTERY_ENC_A, INPUT);
  pinMode(ROTERY_ENC_B, INPUT);
  pinMode(ROTERY_ENC_BUTTON, INPUT);

  digitalWrite(ROTERY_ENC_A, HIGH);  // turn on pull-up resistor
  digitalWrite(ROTERY_ENC_BUTTON, LOW);

//   interrupt to CHANGE,
  attachInterrupt(digitalPinToInterrupt(ROTERY_ENC_A), rotery_turn_interrupt, RISING); // D0 has no interrupt
  attachInterrupt(digitalPinToInterrupt(ROTERY_ENC_BUTTON), rotery_press_interrupt, RISING);

  Serial.begin(9600);
  Serial.println("");
  while (!Serial) {}
  Serial.println("Serial connected");

  tft.begin();
  tft.setBacklight(true);
  tft.setDisplay(true);
  tft.setOrientation(0);
  tft.setFont(Terminal12x16);
  tft.setBackgroundColor(COLOR_BLACK);

  Serial.println("Setup done");
}

// Use arduinojson.org/assistant to compute the capacity.
const size_t capacity = JSON_ARRAY_SIZE(30) + 30 * JSON_OBJECT_SIZE(7) + 245;
DynamicJsonBuffer jsonBuffer(capacity);

void connectToWifi() {
  Serial.printf("Connecting to %s", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }
  Serial.println(", connected");
}

void loop() {
  Serial.println("Loop started");
  tft.setBacklight(true);
  tft.setDisplay(true);
  Serial.printf("start loop, MOTION_DETECTOR_DATA_PIN = %d\n", digitalRead(MOTION_DETECTOR_DATA_PIN));

//  if (next_rotery_counter != prev_rotery_counter) {
//    Serial.printf("Rotery updated to %d\n", next_rotery_counter);
//    prev_rotery_counter = next_rotery_counter;
//  } else {
//    delay(100);
//    return;
//  }

  prev_rotery_counter = next_rotery_counter;
  prev_press_counter = next_press_counter;

  connectToWifi();
  WiFiClient client;
  Serial.printf("Connecting to %s:%d\n", API_SERVER, API_PORT);
  if (!client.connect(API_SERVER, API_PORT))
  {
    Serial.println("Could not connect");
    return;
  }
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

  //tft.drawCircle(tft.maxX()/2, tft.maxY()/2, tft.maxX()/2-1, COLOR_CYAN);
  //  tft.clear();
  tft.drawText(10, 40, "Rotary: " + String(prev_rotery_counter), COLOR_RED);
  tft.drawText(10, 80, "Button: " + String(prev_press_counter), COLOR_CYAN);

  //  delay(1000);
  //  Serial.println("done");

// This will only work with the LED pin attached (this could be connected to the RST of the Wemos)
//  delay(3000);
//  tft.setBacklight(false);
//  tft.setDisplay(false);
//  delay(3000);

   Serial.println("Going to sleep for 20 seconds");
   Serial.printf("pre sleep, MOTION_DETECTOR_DATA_PIN = %d\n", digitalRead(MOTION_DETECTOR_DATA_PIN));
   ESP.deepSleep(20e6); // 3e6 is 20 microseconds
}

void rotery_press_interrupt() {
//  Serial.printf("rotery_press_interrupt, Rotery button pressed: %d\n", digitalRead(ROTERY_ENC_BUTTON));
  next_press_counter++;
}
void rotery_turn_interrupt() {
  // Is called on rising of A => `digitalRead(ROTERY_ENC_A) == true`
//    Serial.printf("Rotery interrupt executed: %d %d\n", digitalRead(ROTERY_ENC_A), digitalRead(ROTERY_ENC_B));
  if (digitalRead(ROTERY_ENC_B)) {
    next_rotery_counter++;
  } else {
    next_rotery_counter--;
  }
}
