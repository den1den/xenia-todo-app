// follwing http://www.bristolwatch.com/arduino/arduino2.htm

#define CHA D3
#define CHB D4

volatile int master_count = 0; // universal count
volatile byte INTFLAG1 = 0; // interrupt status flag

void setup() { 
  pinMode(CHA, INPUT);
  pinMode(CHB, INPUT);

  //pinMode(
  //pinMode(CW_LED, OUTPUT); // LED connected to pin to ground
  //pinMode(CCW_LED, OUTPUT); // LED connected to pin to ground
  
  Serial.begin(9600); 
  Serial.println(master_count);

  // interrupt can be at pn D1..D8 only
  attachInterrupt(D3, flag, RISING);
}

void loop() {
  if (INTFLAG1)   {
    Serial.println(master_count);
    delay(500);
    INTFLAG1 = 0; // clear flag
  } // end if
} // end loop


void flag() {
  INTFLAG1 = 1;
  // add 1 to count for CW
  if (digitalRead(CHA) && !digitalRead(CHB)) {
    master_count++ ;
    //digitalWrite(CW_LED, HIGH);
    //digitalWrite(CCW_LED, LOW);
  }
  // subtract 1 from count for CCW
  if (digitalRead(CHA) && digitalRead(CHB)) {
    master_count-- ;
    //digitalWrite(CW_LED, LOW);
    //digitalWrite(CCW_LED, HIGH);
  } 
}

