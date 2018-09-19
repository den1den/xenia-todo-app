/**
 * An example showing how to put ESP8266 into Deep-sleep mode
 */

#define wake D0
 
void setup() {
  pinMode(wake, OUTPUT);
  digitalWrite(wake, LOW);
  
  Serial.begin(9600);
  Serial.setTimeout(2000);

  // Wait for serial to initialize.
  while(!Serial) { }
  
  Serial.println("");
  Serial.println("I'm awake.");
  delay(3000);

  Serial.println("Going into deep sleep forever");
  ESP.deepSleep(0); // 20e6 is 20 microseconds
}

void loop() {
  delay(10);
}
