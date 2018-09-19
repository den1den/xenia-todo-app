#include <Arduino.h>
#include <ESP8266WiFi.h> # http://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/client-examples.html
#include <ESP8266HTTPClient.h>
#include <TFT_22_ILI9225.h>
#include <ArduinoJson.h> # https://arduinojson.org/
#include <SPI.h>

// SSID of your network
const char WIFI_SSID[] = "dennis"; //SSID of your Wi-Fi router
const char WIFI_PASS[] = "sillybutter708"; //Password of your Wi-Fi router
const int WEMOS_ID = 2;

// Address of the server
const char API_SERVER[] = "dennis.nub.is";
const int API_PORT = 8090;
const char API_ADDRESS[] = "/api/wemos-todos/all";

const char STUPID_PASSWORD[] = "mat6XnqVUPdWeYZFSONJUI7epgScK3";

// #define DEBUG

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

#define DO_WAKE // Uncommet this line to add the deep sleep functionality
#define MOTION_DETECTOR_DATA_PIN D2
#define MOTION_DETECTOR_WAKEUP_PIN D0

#define BUTTON_PRESS_TRESHOLD 10 // We need 10 consegutive presses in order to know its not a debounce 

#define BUTTON_LONG_PRESS_TIME 40 // Time is takes for a long button press (in 0.1 seconds)
#define VIEW_ALL_TODOS_TIMEOUT 50 // Timeout of screen when all todo's are listed but no todo's is selected (in 0.1 seconds) This value is low because then the wemos can go to sleep fast when there was a small motion
#define SELECTED_TODO_TIMEOUT 100 // Timeout of screen when a todo's is selected (in 0.1 seconds)

volatile int next_rotery_counter = 0;
volatile int prev_rotery_counter = 0;
volatile unsigned long last_micros;

TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_SDI, TFT_CLK, TFT_LED, TFT_BRIGHTNESS);

#define TEXT_OFFSET_X 22
#define TEXT_EXTRA_LINES 1
#define TEXT_Y_SPACING 18
int TEXT_OFFSET_Y;
#define TEXT_MAX_CHAR_ON_LINE 25
const int CHAR_WIDTH = 7; // Maximum char width in pixels

int screenOffsetX = 0;
int screenOffsetY = 0;
String CLEAR = "                                    "; // Minimum width of 176/6<30 char is to low

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
  {50, 95}, //10
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

int retrieveTodosResult;

void setup()
{
  retrieveTodosResult = -1; // Start with getting the todo's from the server

  Serial.begin(9600);
  Serial.println();
  //  while (!Serial) {
  //    delay(50);
  //  }
  Serial.println("Serial connected");

#ifdef DO_WAKE
  pinMode(MOTION_DETECTOR_DATA_PIN, INPUT);
  delay(50);
  if (digitalRead(MOTION_DETECTOR_DATA_PIN) == 0) {
    // Wake up because of timeout
    Serial.println("Good morning, there is no motion");
    sleep();
    return;
  }
#endif

  pinMode(ROTERY_ENC_A, INPUT_PULLUP);
  pinMode(ROTERY_ENC_B, INPUT);
  pinMode(ROTERY_ENC_BUTTON, INPUT);

  //  digitalWrite(ROTERY_ENC_A, HIGH);  // turn on pull-up resistor

  // interrupts
  // Note: Only D1..D8 has interrupt
  attachInterrupt(digitalPinToInterrupt(ROTERY_ENC_A), rotery_turn_interrupt, RISING);
  //  attachInterrupt(digitalPinToInterrupt(ROTERY_ENC_BUTTON), rotery_press_interrupt, RISING);

  tft.begin();
  // default: tft.setBackgroundColor(COLOR_BLACK);
  // default: tft.setBacklight(true);
  // default: tft.setDisplay(true);
  tft.setOrientation(1);
  tft.setFont(Terminal6x8); // tft.setFont(Terminal12x16);

  TEXT_OFFSET_Y = tft.maxY() / 2 - 3;

  tft.clear();
  Serial.println("Setup done");
}

// Use arduinojson.org/assistant to compute the capacity.
const size_t capacity = JSON_ARRAY_SIZE(30) + 30 * JSON_OBJECT_SIZE(7) + 245;
TodoData todos[30];
int todos_size = 0;

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
    "Authorization: " + STUPID_PASSWORD + "\r\n" +
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

int i;
void loop() {
  // When the loop startd all TODO's are returned
  Serial.println("Loop started");

  if (retrieveTodosResult != 0) {
    doDrawText("Connecting to Wifi", COLOR_RED);
    connectToWifi();
    doDrawText("Connecting to Server", COLOR_RED);
    retrieveTodosResult = retrieveTodos();
    if (retrieveTodosResult != 0) {
      doDrawText("Could not connect", COLOR_RED);
      Serial.printf("DEBUG retrieveTodos failed with code %d\n", retrieveTodosResult);
      delay(3000);
      return;
    }
    tft.clear();
  }

  int color = COLOR_CYAN;
  //  tft.drawCircle(
  //    tft.maxX() / 2 + screenOffsetX,
  //    tft.maxY() / 2 + screenOffsetY,
  //    (tft.maxY() / 2) - 1,
  //    color);

  // Draw screen
  if (selected == -1) {
    // Idle => show all todo's
    Serial.printf("Show all %d todo's\n", todos_size);
    drawCircles();
    i = 0;
    while (prev_rotery_counter == next_rotery_counter) {
      delay(100);
      i++;
      if ( digitalRead(MOTION_DETECTOR_DATA_PIN) == 1 ) {
        // Cannot go to sleep yet because of PIR
      } else if (i > VIEW_ALL_TODOS_TIMEOUT) {
        // Dont want to go to sleep yet
        Serial.println("VIEW_ALL_TODOS_TIMEOUT");
        sleep();
        return;
      }
    }
    Serial.println("DEBUG going to select todo");
  } else if (selected >= 0) {
    // Selected => show which todo is selected
    // selected is the index of the selected todo in the `todos` array
    Serial.printf("Show selected todo %d\n", selected);
    drawCircles();

    // Wait for input
    i = 0;
    int has_input = 0;
#ifdef DEBUG
    Serial.println("INFO Waiting for input");
#endif
    while (has_input == 0) {

#ifdef DO_WAKE
      if(digitalRead(MOTION_DETECTOR_DATA_PIN) == 0){
        Serial.printf("WARNING When motion is detected the device will restart");
      }
#endif
      
      if (digitalRead(ROTERY_ENC_BUTTON) == 0) {
        // Check if button is really pressed
        int j = 0;
        while (++j <= BUTTON_PRESS_TRESHOLD && (digitalRead(ROTERY_ENC_BUTTON) == 0)) {
          delay(1);
#ifdef DEBUG
          Serial.println("INFO button pressed?");
#endif
        }
        if (digitalRead(ROTERY_ENC_BUTTON) == 0) {
#ifdef DEBUG
          Serial.println("INFO button really pressed!");
#endif
          has_input = 1;
          break;
        }
      }
      if (prev_rotery_counter != next_rotery_counter) {
#ifdef DEBUG
        Serial.println("INFO rotery turned");
#endif
        has_input = 2;
      }
      // wait for a while to check for input again
      delay(100);
      i++;
      if (i >= SELECTED_TODO_TIMEOUT) {
        // For a very long time nothing happened
#ifdef DEBUG
        Serial.println("INFO Waiting for input, timeout");
#endif
        selected = -1;
        tft.clear();
        return;
      }
    }

    // Input has happend
    if (has_input == 1) {
      // Button is pressed (briefly)
      doDrawTextFull(todos[selected].name);

      i = 0;
      while (digitalRead(ROTERY_ENC_BUTTON) == 0) {
        delay(100);
        i++;
        if (i >= BUTTON_LONG_PRESS_TIME) {
#ifdef DEBUG
          Serial.printf("Done todo %s\n", todos[selected].name);
#endif
          doClearTextFull();
          drawCircles();
          doDrawText("Done", COLOR_WHITE);

          int sendTodoResult = sendDone(selected);
          Serial.printf("sendTodoResult=%d\n", sendTodoResult);
          if (sendTodoResult != 0) {
            tft.clear();
            doDrawText("Error", COLOR_RED);
            Serial.printf("DEBUG sendTodo done failed with code %d\n", sendTodoResult);
            delay(3000);
            return;
          }

          // Remove todo from screen
          todos[selected].status = "-1";
          Serial.print("Todo removed");

          selected = -1;
          Serial.print("Clearing");
          tft.clear();
          Serial.println(".");

          return;
        }
      }
      // Button is not pressed more then 5 seconds
      doClearTextFull();
      // Stay selected and draw everything again
      return;
    } else {
      // There has been a rotery change
      // Then pick next selection, just as in home screen
    }
  }

  //  i = 0;
  //  Serial.printf("end of loop, next_press_counter: %d\n", next_press_counter);
  //  while(prev_rotery_counter == next_rotery_counter && prev_press_counter == next_press_counter) {
  //    // Waiting for input
  //    delay(200);
  //    if(i++ >= 25){
  //      sleep();
  //      return;
  //    }
  //  }
  if (prev_rotery_counter != next_rotery_counter) {
    // Rotery encoder is turned
    int selection_change = next_rotery_counter - prev_rotery_counter;
    prev_rotery_counter = next_rotery_counter;
    Serial.printf("Rotery encoder changed: %d\n", selection_change);

    if (todos_size == 0) {
      return;
    }

    if (selection_change > 0) {
      selection_change = 1;
    } else {
      selection_change = -1;
    }

    while (true) {
      selected += selection_change;
      while ( selected < 0 ) {
        selected += todos_size;
      }
      selected %= todos_size;
      if (todos[selected].status != "-1") break;
    }

    return; // Redraw
  } else {
    // Button press on overview screen
    // Indirectly resets the timeout timer
    Serial.println("Button press on overview screen");
  }
}

void rotery_turn_interrupt() {
  // Is called on rising of A => `digitalRead(ROTERY_ENC_A) == true`
  // Serial.printf("Rotery interrupt executed: %d %d\n", digitalRead(ROTERY_ENC_A), digitalRead(ROTERY_ENC_B));
  if (analogRead(ROTERY_ENC_B) > 512) {
    next_rotery_counter++;
  } else {
    next_rotery_counter--;
  }
}

void doClearTextFull() {
  drawText(TEXT_OFFSET_X, TEXT_OFFSET_Y, CLEAR, COLOR_WHITE);
  for (int j = 1; j <= TEXT_EXTRA_LINES; j++) {
    //    Serial.printf("TEXT_OFFSET_Y=%d, ", TEXT_OFFSET_Y);
    //    Serial.printf("i=%d, ", j);
    //    Serial.printf("TEXT_Y_SPACING=%d, ", TEXT_Y_SPACING);
    //    Serial.printf("i * TEXT_Y_SPACING=%d, ", j * TEXT_Y_SPACING);
    //    Serial.printf("TEXT_OFFSET_Y + j * TEXT_Y_SPACING=%d, ", TEXT_OFFSET_Y + j * TEXT_Y_SPACING);
    drawText(TEXT_OFFSET_X, TEXT_OFFSET_Y + j * TEXT_Y_SPACING, CLEAR, COLOR_WHITE);
    drawText(TEXT_OFFSET_X, TEXT_OFFSET_Y - j * TEXT_Y_SPACING, CLEAR, COLOR_WHITE);
  }
}

void drawText(int x, int y, String s, int color) {
  tft.drawText(x, y, s, color);
#ifdef DEBUG
  Serial.printf("Printing `%s` at (%d, %d)\n", s.c_str(), x, y);
#endif
}

void doDrawTextFull(String s) {
  int y;
  for (int j = 0; j < TEXT_EXTRA_LINES; j++) {
    y = TEXT_OFFSET_Y - (TEXT_EXTRA_LINES - j) * TEXT_Y_SPACING;
    _doDrawText(s.substring(j * TEXT_MAX_CHAR_ON_LINE, (j + 1)*TEXT_MAX_CHAR_ON_LINE), COLOR_WHITE, y);
  }
  for (int j = 0; j <= TEXT_EXTRA_LINES; j++) {
    y = TEXT_OFFSET_Y + j * TEXT_Y_SPACING;
    _doDrawText(
      s.substring((TEXT_EXTRA_LINES + j)*TEXT_MAX_CHAR_ON_LINE, (TEXT_EXTRA_LINES + j + 1)*TEXT_MAX_CHAR_ON_LINE),
      COLOR_WHITE, y
    );
  }
}

void doDrawText(String s, uint16_t color) {
  _doDrawText(s, color, TEXT_OFFSET_Y);
}

void _doDrawText(String s, uint16_t color, int y) {
  int x = TEXT_OFFSET_X;
  drawText(x, y, CLEAR, COLOR_WHITE);
  if (s.length() <= TEXT_MAX_CHAR_ON_LINE) {
    x += (TEXT_MAX_CHAR_ON_LINE - s.length()) * CHAR_WIDTH / 2; // center
    drawText(x, y, s, color);
  } else {
    s = s.substring(0, TEXT_MAX_CHAR_ON_LINE - 4) + " ...";
    drawText(x, y, s, color);
  }
}

uint16_t getTodoColor(boolean priority, boolean local, boolean selected) {
  if (selected) {
    if (priority) return COLOR_RED;
    if (local) return COLOR_GREEN;
    return COLOR_WHITE;
  }
  if (priority) return COLOR_DARKRED;
  if (local) return COLOR_DARKGREEN;
  return COLOR_LIGHTGREY;
}

int sendDone(int selected) {
  WiFiClient client;

  if (client.status() != WL_CONNECTED) {
    Serial.printf("Connecting to %s:%d\n", API_SERVER, API_PORT);
    if (!client.connect(API_SERVER, API_PORT))
    {
      Serial.println("Could not connect");
      return 1;
    }
  }

  String REQUEST = String("DELETE ") + API_ADDRESS + "/" + String(todos[selected].id) + "?wemos=" + String(WEMOS_ID) + " HTTP/1.1\r\n" +
                   "Host: " + API_SERVER + "\r\n" +
                   "Connection: close\r\n" +
                   "Authorization: " + STUPID_PASSWORD + "\r\n" +
                   "\r\n";
  Serial.println("Sending request:");
  Serial.print(REQUEST);
  client.print(REQUEST);

  // Check HTTP status
  Serial.print("Check HTTP status:");
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 204 No Content") != 0) {
    Serial.print("Unexpected response: ");
    Serial.println(status);
    return 2;
  }
  Serial.println(" Ok");

  while (client.available()) {
    char c = client.read();
    Serial.print(c);
  }
  Serial.flush();

  Serial.println("Stopping client");
  client.stop();
  return 0;
}

void drawCircles() {
  //  Serial.printf("INFO: drawCircles(selected=%d)\n", selected);
  //  Serial.printf("INFO: drawCircles(selected=%d, todos[selected].status=%d, todos[selected].id)\n", selected, todos[selected].status, todos[selected].id);
  uint16_t selected_color;
  for (int i = 0; i < todos_size; i++) {
    if (!String(todos[i].status).equals("0")) {
      continue;
    }
    boolean priority = String(todos[i].priority).equals("1");
    boolean local = (todos[i].location == WEMOS_ID);
    uint16_t color;
    if (i == selected) {
      color = getTodoColor(priority, local, true);
      selected_color = color;
    } else {
      color = getTodoColor(priority, local, false);
    }
    tft.fillCircle(circlePositions[i][0] + 22 + screenOffsetX, circlePositions[i][1] - 22 + screenOffsetY, 5, color);
  }
  if (selected >= 0) {
    if (!String(todos[selected].status).equals("0")) {
      Serial.println("ERROR: drawing circles with a selected circle which is deleted");
    } else {
      doDrawText(todos[selected].name, selected_color);
    }
  }
}

void sleep() {
#ifdef DO_WAKE
  if (digitalRead(MOTION_DETECTOR_DATA_PIN) == 1) {
    Serial.print("Wanting to go to sleep ");
    while (digitalRead(MOTION_DETECTOR_DATA_PIN) == 1) {
      delay(100);
      Serial.print(".");
    }
    Serial.println();
  }
  Serial.println("Goodnight");
  ESP.deepSleep(10e6); // 3e6 is 20 microseconds
#endif
}
