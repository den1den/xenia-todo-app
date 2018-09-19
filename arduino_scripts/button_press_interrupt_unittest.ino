#define ROTERY_ENC_BUTTON D3
//#define ROTERY_ENC_A D4

long debouncing_time = 100; //Debouncing Time in Milliseconds
volatile unsigned long last_micros;
volatile unsigned int button_count = 0;

void setup() {
  Serial.begin(9600);
  Serial.println();
  while (!Serial) {
    delay(50);
  }

  pinMode(ROTERY_ENC_BUTTON, INPUT);
//  pinMode(ROTERY_ENC_A, INPUT);
  attachInterrupt(digitalPinToInterrupt(ROTERY_ENC_BUTTON), rp_interrupt, RISING);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("looperdy loop");
  Serial.printf("ROTERY_ENC_BUTTON=%d\n", digitalRead(ROTERY_ENC_BUTTON));
  Serial.printf("button_count=%d\n", button_count);
  delay(1000);
}

void rp_interrupt() {
  if((long)(micros() - last_micros) >= debouncing_time * 1000) {
    button_count++;
    last_micros = micros();
  }
}

