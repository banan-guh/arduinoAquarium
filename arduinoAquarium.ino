const int threshold = 900;
const int greenLED = 3;
const int redLED =   4;
// Too high floats: A5 (upper), A7 (lower)
// Too low floats:  A1 (upper), A# (lower)
const int pinHighTop =    A5;
const int pinHighBottom = A7;
//const int pinLowTop =     A1;
//const int pinLowBottom =  A#;

void do_LED(bool isOK) {
  if (isOK) {
    digitalWrite(greenLED, HIGH);
    digitalWrite(redLED, LOW);
  }
  else {
    digitalWrite(greenLED, LOW);
    digitalWrite(redLED, HIGH);
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  int readOne = digitalRead(A5);
  int readTwo = digitalRead(A7);
  //int readThree = analogRead();
  //int readFour = analogRead();

  bool isOneTriggered = (readOne < threshold);
  bool isTwoTriggered = (readTwo < threshold);

  bool isOK = true;
  
  if (isOneTriggered or isTwoTriggered) {
    isOK = false;
  }

  do_LED(isOK);
}
