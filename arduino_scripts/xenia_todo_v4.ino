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

#define ROTERY_ENC_A D4 // D4
#define ROTERY_ENC_B A0
#define ROTERY_ENC_BUTTON D3

#define TFT_RST D1 // of the screen on RST of Wemos
#define TFT_RS  D6
#define TFT_CS  NO_PIN  // SS
#define TFT_SDI D7 // MOSI
#define TFT_CLK D5 // SCK
#define TFT_LED D8
#define TFT_BRIGHTNESS 255 // Initial brightness of TFT backlight (optional)

// #define DO_WAKE // Uncommet this line to add the deep sleep functionality
#define MOTION_DETECTOR_DATA_PIN D3
#define WAKEUP_PIN D0

volatile int next_rotery_counter = 0;
volatile int prev_rotery_counter = 0;

volatile int next_press_counter = 0;
volatile int prev_press_counter = 0;
volatile boolean button_pressed = false;
TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_SDI, TFT_CLK, TFT_LED, TFT_BRIGHTNESS);

int circlePositions[][30] = {
  {65, 80}, //1
  {130, 130}, //2
  {75, 120}, //3
  {40, 125}, //4
  {75, 100}, //5
  {100, 70}, //6
  {120, 100}, //7
  {100, 150}, //8
  {40, 80}, //9
  {50,95}, //10
  {100, 120}, //11
  {80, 165}, //12
  {100, 100}, //13
  {20, 95}, //14
  {75, 60}, //15
  {125, 70}, //16
  {155, 120}, //17
  {145, 150}, //18
  {125, 175}, //19
  {40, 150}, //20
  {45, 55}, //21
  {80, 40}, //22
  {150, 65}, //23
  {150, 95}, //24
  {100, 180}, //25
  {55, 175}, //26
  {20, 125}, //27
  {70, 145}, //28
  {125, 150}, //29
  {115, 50} //30
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

  pinMode(ROTERY_ENC_A, INPUT_PULLUP);
  pinMode(ROTERY_ENC_B, INPUT);
//  pinMode(ROTERY_ENC_BUTTON, INPUT_PULLUP);

//  digitalWrite(ROTERY_ENC_A, HIGH);  // turn on pull-up resistor

  // interrupts
  // Note: Only D1..D8 has interrupt
  attachInterrupt(digitalPinToInterrupt(ROTERY_ENC_A), rotery_turn_interrupt, RISING);
//  attachInterrupt(digitalPinToInterrupt(ROTERY_ENC_BUTTON), rotery_press_interrupt, RISING);
//  attachInterrupt(digitalPinToInterrupt(ROTERY_ENC_BUTTON), rotery_press_interrupt, FALLING);

  Serial.begin(9600);
  Serial.println();
  while (!Serial) {
    delay(50);
  }
  Serial.println("Serial connected");

  tft.begin();
  // default: tft.setBackgroundColor(COLOR_BLACK);
  // default: tft.setBacklight(true);
  // default: tft.setDisplay(true);
  tft.setOrientation(1);
  tft.setFont(Terminal6x8); // tft.setFont(Terminal12x16);
  
  tft.clear();
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

int retrieveTodos() {
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

  if (retrieveTodosResult != 0) {
    tft.drawText(0, tft.maxY() / 2, "Conn. to Wifi");
    connectToWifi();
    tft.drawText(0, tft.maxY() / 2, "Conn. to Server");
    retrieveTodosResult = retrieveTodos();
    if (retrieveTodosResult != 0) {
      tft.clear();
      tft.drawText(0, tft.maxY() / 2, "Error");
      Serial.printf("DEBUG retrieveTodos failed with code %d\n", retrieveTodosResult);
      delay(3000);
      return;
    }
    tft.clear();
  }

  int color = COLOR_CYAN;
  tft.drawCircle(tft.maxX() / 2, tft.maxY() / 2, tft.maxY() / 2 - 1, color);

  if (selected == -1) {
    // Idle => show all todo's
    Serial.printf("Show all %d todo's\n", todos_size);
    for (int i = 0; i < todos_size; i++) {
      boolean priority = String(todos[i].priority).equals("1");
      boolean local = todos[i].location > 1;
      color = getTodoColor(priority, local, false);
      tft.fillCircle(circlePositions[i][0]+22, circlePositions[i][1]-22, 5, color);
    }
  } else if (selected >= 0) {
    // Selected => show which todo is selected
    // selected is the index of the selected todo in the `todos` array
    Serial.printf("Show selected todo %d\n", selected);
    uint16_t selected_color;
    for (int i = 0; i < todos_size; i++) {
      boolean priority = String(todos[i].priority).equals("1");
      boolean local = todos[i].location > 1;
      if (i == selected) {
        color = getTodoColor(priority, local, true);
        selected_color = color;
      } else {
        color = getTodoColor(priority, local, false);
      }
      tft.fillCircle(circlePositions[i][0]+22, circlePositions[i][1]-22, 5, color);
    }
    doDrawText(todos[selected].name, selected_color);

    int i = 0;
    while(prev_rotery_counter == next_rotery_counter && prev_press_counter == next_press_counter) {
      // Waiting for input
      delay(200);
      i++;
      if(i >= 25) {
        selected = -1;
        Serial.println("timeout");
        tft.clear();
        Serial.println();
        return;
      }
    }
    if(prev_press_counter != next_press_counter){
      // Button is pressed
      doDrawTextFull(todos[selected].name);
      i = 0;
      while(button_pressed){
        delay(200);
        i++;
        if(i >= 25){
          // Pressed for more then 5 seconds
          Serial.printf("Done todo %s\n", todos[selected].name);
          selected = -1;
          tft.clear();
        }
      }
      // Button is released
      return;
    }
  }

  while(prev_rotery_counter == next_rotery_counter && prev_press_counter == next_press_counter) {
    // Waiting for input
    delay(200);
  }
  if (prev_rotery_counter != next_rotery_counter) {
    // Rotery encoder is turned
    int selection_change = next_rotery_counter - prev_rotery_counter;
    prev_rotery_counter = next_rotery_counter;
    Serial.printf("Rotery encoder changed: %d\n", selection_change);

    if (todos_size == 0) {
      return;
    }
    
    if(selection_change > 0){
      selection_change = 1;
    } else {
      selection_change = -1;
    }
    
    selected += selection_change;
    while ( selected < 0 ) {
      selected += todos_size;
    }
    selected %= todos_size;

    return;
  }
  if (prev_press_counter != next_press_counter) {
    prev_press_counter = next_press_counter;
    Serial.print("Button pressed");
    delay(1000);
    Serial.println(".");
    return;
  }
#ifdef DO_WAKE
  Serial.println("Going to sleep for 20 seconds");
  Serial.printf("pre sleep, MOTION_DETECTOR_DATA_PIN = %d\n", digitalRead(MOTION_DETECTOR_DATA_PIN));
  ESP.deepSleep(20e6); // 3e6 is 20 microseconds
#endif
}

void rotery_press_interrupt() {
  Serial.printf("rotery_press_interrupt: ROTERY_ENC_BUTTON=%d\n", digitalRead(ROTERY_ENC_BUTTON));
  next_press_counter++;
  button_pressed = true;
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

void doDrawTextFull(String s){
  tft.drawText(0, tft.maxY() / 2 - 18, s.substring(0, 16));
  tft.drawText(0, tft.maxY() / 2, s.substring(16, 32));
  tft.drawText(0, tft.maxY() / 2 + 18, s.substring(32));
}

void doDrawText(String s, uint16_t color){
  int x = 22;
  int y = tft.maxY() / 2 - 4 + 1;
  int maxStringSize = 25;
  int CHAR_WIDTH = 7; // Maximum char width is 7
  String CLEAR = "                                    "; // Minimum width of 176/6<30 char is to low
  tft.drawText(x, y, CLEAR);
  if(s.length() <= maxStringSize){
    x += (maxStringSize - s.length()) * CHAR_WIDTH / 2; // center
    tft.drawText(x, y, s, color);
  } else {
    s = s.substring(0, maxStringSize - 4) + " ...";
    tft.drawText(x, y, s, color);
  }
//  if(s.length() == maxStringSize) {
//    
//    Serial.println("perfect");
//    tft.drawText(x, y, CLEAR);
//    tft.drawText(x, y, s);
//    
//  } else if (s.length() < maxStringSize) {
//    
//    Serial.println("triming");
//    int missingChars = maxStringSize - s.length();
//    String rs = "";
//    for(int i = 0; i < missingChars/2 + 4; i++){
//      //rs += " ";
//    }
//    rs += s;
//    for(int i = 0; i < missingChars/2 + 7; i++){
//      //rs += " ";
//    }
//    tft.drawText(x, y, CLEAR);
//    tft.drawText(x, y, s);
////    tft.drawText(0, tft.maxY() / 2 - 18, "                                ");
////    tft.drawText(0, tft.maxY() / 2 + 18, "                                ");
////  } else if (s.length() <= 32) {
////    tft.drawText(0, tft.maxY() / 2 - 18, s.substring(0, 16));
////    tft.drawText(0, tft.maxY() / 2, s.substring(16) + "                                ");
////    tft.drawText(0, tft.maxY() / 2 + 18, "                                ");
//  
//  } else {
//    
//    Serial.println("truncating");
//    tft.drawText(0, tft.maxY() / 2 - 18, s.substring(0, 16));
//    s = s.substring(0, maxStringSize + 1 - 4) + " ...";
//    tft.drawText(x, y, CLEAR);
//    tft.drawText(x, y, s);
//    tft.drawText(0, tft.maxY() / 2 + 18, s.substring(32) + "                                ");
//  }
//  tft.drawLine(0, y-1, tft.maxX() + 1, y-1, COLOR_GREEN);
//  tft.drawLine(0, y-2, tft.maxX() + 1, y-1, COLOR_GREEN);
//  tft.drawLine(0, y-1, s.length() * (CHAR_WIDTH - 1), y-1, COLOR_WHITE);
//  tft.drawLine(0, y-1, s.length() * (CHAR_WIDTH), y-1, COLOR_RED);
  //  tft.drawText(0, tft.maxY() / 2 - 18, "                ");
  //  tft.drawText(0, tft.maxY() / 2, "                ");
  //  tft.drawText(0, tft.maxY() / 2 + 18, "                ");
}

uint16_t getTodoColor(boolean priority, boolean local, boolean selected) {
  if(selected){
    if(priority) return COLOR_RED;
    if(local) return COLOR_GREEN;
    return COLOR_WHITE;
  }
  if(priority) return COLOR_DARKRED;
  if(local) return COLOR_DARKGREEN;
  return COLOR_LIGHTGREY;
}
