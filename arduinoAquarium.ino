const int threshold = 900;
const int greenLED = 3;
const int redLED = 4;
// A5, A7


void do_LED(bool isOK) {
  if (isOK) {
    digitalWrite(greenLED, HIGH);
    // green light is on
    digitalWrite(redLED, LOW);
    // red light is off
  }
  else {
    digitalWrite(greenLED, LOW);
    // green light is off
    digitalWrite(redLED, HIGH);
    //red light is on
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  int readOne = analogRead(A5);
  int readTwo = analogRead(A7);
  
  bool isOneTriggered = (readOne < threshold);
  bool isTwoTriggered = (readTwo < threshold);

  bool isOK = true;
  
  if (isOneTriggered or isTwoTriggered) {
    isOK = false;
  }

  do_LED(isOK);
}
//I was here
// no one cares
