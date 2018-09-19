#define MOTION_DETECTOR_DATA_PIN D2
#define MOTION_DETECTOR_WAKEUP_PIN D0

void setup() {
  Serial.begin(9600);
  Serial.println("");
  
  pinMode(MOTION_DETECTOR_DATA_PIN, INPUT);
  delay(25);
  int startup_read = digitalRead(MOTION_DETECTOR_DATA_PIN);
  Serial.printf("startup_read=%d\n", startup_read);

  if (startup_read == LOW) {
    // Wake up not because of movement
    Serial.println("Good morning, there is no motion");
    sleep();
    return;
  } else {
    Serial.println("Hello you there");
  }

  digitalWrite(MOTION_DETECTOR_DATA_PIN, HIGH);

  Serial.print("Do some importnat stuff that will end with DONE:\nworking");
  int motion = -1;
  for(int i = 0; i < 2*10; i++){
    if(motion != digitalRead(MOTION_DETECTOR_DATA_PIN)){
      motion = digitalRead(MOTION_DETECTOR_DATA_PIN);
      Serial.printf("MOTION=%d", motion);
    }
    Serial.print(".");
    delay(500);
  }
  Serial.println("DONE");

  sleep();
}

void loop() {
}

void sleep() {
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
}
