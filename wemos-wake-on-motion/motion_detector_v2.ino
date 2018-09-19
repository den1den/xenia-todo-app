#define data D2
#define wake D0

void setup() {
  Serial.begin(19200);
  Serial.println("");
  Serial.println("I'm awake");
  
  pinMode(data, INPUT);
  delay(50);
//  pinMode(wake, OUTPUT);
//  digitalWrite(wake, LOW);

  if(digitalRead(data)){
    Serial.println("I woke up because of PIR");
  } else {
    Serial.println("I woke up because of time out");
  }

  delay(3000);
  Serial.print("Wanting to go to sleep: ");
  while(digitalRead(data)){
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Going into deep sleep for 2 seconds");
  ESP.deepSleep(2e6);
//  ESP.deepSleep(0);
}

void loop() {
}
