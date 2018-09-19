#include <Arduino.h>
#include <ESP8266WiFi.h> # http://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/client-examples.html
#include <ESP8266HTTPClient.h>
#include <TFT_22_ILI9225.h>
#include <ArduinoJson.h> # https://arduinojson.org/
#include <SPI.h>

// SSID of your network
const char WIFI_SSID[] = "dennis"; //SSID of your Wi-Fi router
const char WIFI_PASS[] = "sillybutter708"; //Password of your Wi-Fi router
const int WEMOS_ID = 1;

// Address of the server
const char API_SERVER[] = "192.168.1.50";
const int API_PORT = 7777;
const char API_ADDRESS[] = "/api/wemos-todos/all";

#define NO_PIN 0

#define ROTERY_ENC_A D4
#define ROTERY_ENC_B D1
#define ROTERY_ENC_BUTTON D2

#define TFT_RST D0 // of the screen on RST of Wemos
#define TFT_RS  D6
#define TFT_CS  NO_PIN  // SS
#define TFT_SDI D7 // MOSI
#define TFT_CLK D5 // SCK
#define TFT_LED D8
#define TFT_BRIGHTNESS 200 // Initial brightness of TFT backlight (optional)

// #define DO_WAKE // Uncommet this line to add the deep sleep functionality
#define MOTION_DETECTOR_DATA_PIN D3
#define WAKEUP_PIN D0

volatile int next_rotery_counter = 0;
volatile int prev_rotery_counter = 0;
volatile int next_press_counter = 0;
volatile int prev_press_counter = 0;
TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_SDI, TFT_CLK, TFT_LED, TFT_BRIGHTNESS);

int circlePositions[][30] = {
  {65,80},  //1
  {130,130},//2
  {75,120}, //3
  {40,125}, //4
  {75,100}, //5
  {100,70}, //6
  {120,100},//7 
  {100,150},//8
  {40,80},  //9
  {50,95}, //10
  {100,120},//11
  {80,165}, //12
  {100,100},//13
  {20,95},  //14
  {75,60},  //15
  {125,70}, //16
  {155,120},//17
  {145,150},//18
  {125,175},//19
  {40,150}, //20
  {45,55},  //21
  {80,40},  //22
  {150,65}, //23
  {150,95}, //24
  {100,180},//25
  {55,175}, //26
  {20,125}, //27
  {70,145}, //28
  {125,150},//29
  {115,50}  //30
};

typedef struct {
  int id;
  const char* name;
  int location;
  const char* priority;
  const char* status;
} TodoData;

void setup()
{
  #ifdef DO_WAKE
  pinMode(WAKEUP_PIN, INPUT);
  digitalWrite(WAKEUP_PIN, LOW);
  pinMode(MOTION_DETECTOR_DATA_PIN, INPUT);
  #endif
  
  pinMode(ROTERY_ENC_A, INPUT);
  pinMode(ROTERY_ENC_B, INPUT);
  pinMode(ROTERY_ENC_BUTTON, INPUT);

  digitalWrite(ROTERY_ENC_A, HIGH);  // turn on pull-up resistor
  digitalWrite(ROTERY_ENC_BUTTON, LOW);
  
  // interrupts
  // Note: D0 has no interrupt
  attachInterrupt(digitalPinToInterrupt(ROTERY_ENC_A), rotery_turn_interrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(ROTERY_ENC_BUTTON), rotery_press_interrupt, RISING);

  Serial.begin(9600);
  Serial.println("");
  while (!Serial) {delay(1);}
  Serial.println("Serial connected");

  tft.begin();
  tft.clear();
  tft.setBacklight(true);
  tft.setDisplay(true);
  tft.setOrientation(0);
  tft.setFont(Terminal12x16); //tft.setFont(Terminal6x8);
  tft.setBackgroundColor(COLOR_BLACK);
  
  Serial.println("Setup done");
}

// Use arduinojson.org/assistant to compute the capacity.
const size_t capacity = JSON_ARRAY_SIZE(30) + 30 * JSON_OBJECT_SIZE(7) + 245;
TodoData todos[30];
int todos_size = 0;

int retrieveTodosResult = -1; // Start with getting the todo's from the server
int selected = -1;

void connectToWifi() {
  Serial.printf("Connecting to %s ", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println(" connected");
}


int retrieveTodos(){
  WiFiClient client;
  Serial.printf("Connecting to %s:%d\n", API_SERVER, API_PORT);
  if (!client.connect(API_SERVER, API_PORT))
  {
    Serial.println("Could not connect");
    return 1;
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
    return 2;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println("Invalid response");
    return 3;
  }

  // Parse JSON object
  DynamicJsonBuffer jsonBuffer(capacity);
  JsonArray& root = jsonBuffer.parseArray(client);
  if (!root.success()) {
    Serial.println("Parsing failed!");
    return 4;
  }
  todos_size = root.size();
  Serial.printf("DEBUG todos_size=%d\n", todos_size);
  todos_size %= 30;
  for (int i = 0; i < todos_size; i++) {
    JsonObject& todoJson = root.get<JsonObject>(i);
    TodoData todo;
    todo.id = todoJson["id"];
    todo.name = todoJson["name"];
    todo.location = todoJson["location"];
    todo.priority = todoJson["priority"];
    todo.status = todoJson["status"];
    todos[i] = todo;
    Serial.printf("DEBUG stored debug %d\n", i);
  }
  client.stop();
  return 0;
}

void loop() {
  Serial.println("Loop started");

  #ifdef DO_WAKE
  Serial.printf("start loop, MOTION_DETECTOR_DATA_PIN = %d\n", digitalRead(MOTION_DETECTOR_DATA_PIN));
  #endif
  
//  if (next_rotery_counter != prev_rotery_counter) {
//    Serial.printf("Rotery updated to %d\n", next_rotery_counter);
//    prev_rotery_counter = next_rotery_counter;
//  } else {
//    delay(100);
//    return;
//  }

  if(retrieveTodosResult != 0){
    connectToWifi();
    retrieveTodosResult = retrieveTodos();
    if(retrieveTodosResult != 0){
      Serial.printf("DEBUG retrieveTodos failed with code %d\n", retrieveTodosResult);
      delay(5000);
      return;
    }
  }

//  tft.clear();
  int color = COLOR_CYAN;
  tft.drawCircle(tft.maxX()/2, tft.maxY()/2, tft.maxX()/2-1, color);
  
  if(selected == -1){
    // Idle => show all todo's
    Serial.printf("Show all %d todo's\n", todos_size);
    for (int i = 0; i < todos_size; i++){
      color = COLOR_WHITE;
      tft.fillCircle(circlePositions[i][0],circlePositions[i][1], 5, color);
    }
    Serial.println(".");
  } else if (selected >= 0) {
    // Selected => show which todo is selected
    // selected is the index of the selected todo in the `todos` array
    Serial.printf("Show selected todo %d\n", selected);
    for (int i = 0; i < todos_size; i++){
      if(i == selected){
        color = COLOR_CYAN;
      } else {
        color = COLOR_WHITE;
      }
      tft.fillCircle(circlePositions[i][0],circlePositions[i][1], 5, color);
    }
    Serial.printf("DEBUG selected todo name is %s\n", todos[selected].name);
    tft.drawText(0, tft.maxY()/2, todos[selected].name);
    Serial.println(".");
  }
  delay(300);
  
  if (prev_rotery_counter != next_rotery_counter) {
    // Rotery encoder is turned
    int selection_change = next_rotery_counter - prev_rotery_counter;
    prev_rotery_counter = next_rotery_counter;

    if(todos_size == 0){
      return;
    }
    selected += selection_change;
    while ( selected < 0 ) {
      selected += todos_size;
    }
    selected %= todos_size;
    
    Serial.printf("Rotery encoder changed: %d\n", selection_change);
    return;
  }

  if (prev_press_counter != next_press_counter) {
    prev_press_counter = next_press_counter;
    Serial.print("Button pressed");
    delay(1000);
    Serial.println(".");
    return;
  }
  
//  Serial.print("Todo[0] = ");
//  JsonObject& firstTodo = todos.get<JsonObject>(0);
//  int firstId = firstTodo["id"];
//  const char* firstName = firstTodo["name"];
//  Serial.printf("%d: %s", firstId, firstName);
//  Serial.println("");

  //tft.drawCircle(tft.maxX()/2, tft.maxY()/2, tft.maxX()/2-1, COLOR_CYAN);
  //  tft.clear();
//  tft.drawText(10, 40, "Rotary: " + String(prev_rotery_counter), COLOR_RED);
//  tft.drawText(10, 80, "Button: " + String(prev_press_counter), COLOR_CYAN);

  //  delay(1000);
  //  Serial.println("done");

// This will only work with the LED pin attached (this could be connected to the RST of the Wemos)
//  delay(3000);
//  tft.setBacklight(false);
//  tft.setDisplay(false);
//  delay(3000);
  
  #ifdef DO_WAKE
  Serial.println("Going to sleep for 20 seconds");
  Serial.printf("pre sleep, MOTION_DETECTOR_DATA_PIN = %d\n", digitalRead(MOTION_DETECTOR_DATA_PIN));
  ESP.deepSleep(20e6); // 3e6 is 20 microseconds
  #endif
}

void rotery_press_interrupt() {
  // Serial.printf("rotery_press_interrupt, Rotery button pressed: %d\n", digitalRead(ROTERY_ENC_BUTTON));
  next_press_counter++;
}

void rotery_turn_interrupt() {
  // Is called on rising of A => `digitalRead(ROTERY_ENC_A) == true`
  // Serial.printf("Rotery interrupt executed: %d %d\n", digitalRead(ROTERY_ENC_A), digitalRead(ROTERY_ENC_B));
  if (digitalRead(ROTERY_ENC_B)) {
    next_rotery_counter++;
  } else {
    next_rotery_counter--;
  }
}
