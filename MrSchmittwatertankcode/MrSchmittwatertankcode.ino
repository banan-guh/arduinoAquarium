
const int greenLED = 3;
const int redLED =   4;
const int floatOne = 5;
const int floatTwo = 6;
const int MotorHigh = 11;
const int MotorLow = 12;
const int Button = 7;
const int Relay = 9;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(greenLED, OUTPUT);
  pinMode(redLED,   OUTPUT);
  pinMode(MotorHigh, OUTPUT);
  pinMode(MotorLow, OUTPUT);
  pinMode(Relay, OUTPUT);
  
  pinMode(floatTwo, INPUT_PULLUP);
  pinMode(floatOne, INPUT_PULLUP);
  pinMode(Button, INPUT_PULLUP);

}

void loop() {
  // put your main code here, to run repeatedly:
  int readOne = digitalRead(floatOne);
  int readTwo = digitalRead(floatTwo);
  int ButtonRead = digitalRead(Button);
  bool isFloatOneUp = (readOne == LOW);
  bool isFloatTwoUp = (readTwo == LOW);
  bool isButtonOn = (ButtonRead == LOW);
  //Serial.println(isFloatOneUp);
  //Serial.println(isFloatTwoUp);
 
  if (isButtonOn) {
    digitalWrite(Relay, HIGH);
    digitalWrite(MotorHigh, HIGH);
    digitalWrite(MotorLow, LOW);
  }
  else {
    digitalWrite(MotorHigh, LOW);
    digitalWrite(MotorLow, LOW);
    digitalWrite(Relay, LOW);
  }
  
  if (isFloatOneUp and isFloatTwoUp) {
    digitalWrite(redLED,  HIGH);
    digitalWrite(greenLED, LOW); 
  }
  else {
    digitalWrite(redLED,   LOW);
    digitalWrite(greenLED, HIGH);
  } 
}
