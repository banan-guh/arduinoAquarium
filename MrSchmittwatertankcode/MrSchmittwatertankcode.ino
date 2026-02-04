
const int greenLED = 3;
const int redLED =   4;
const int floatOne = 5;
const int floatTwo = 6;
const int MotorHigh = 11;
const int MotorLow = 12;
const int button = 7;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(greenLED, OUTPUT);
  pinMode(redLED,   OUTPUT);
  pinMode(MotorHigh, OUTPUT);
  pinMode(MotorLow, OUTPUT);
  
  pinMode(floatTwo, INPUT_PULLUP);
  pinMode(floatOne, INPUT_PULLUP);
  pinMode(button, INPUT_PULLUP);
}

void loop() {
  // put your main code here, to run repeatedly:
  int readOne = digitalRead(floatOne);
  int readTwo = digitalRead(floatTwo);
  int buttonread = digitalRead(button);
  bool isFloatOneUp = (readOne == LOW);
  bool isFloatTwoUp = (readTwo == LOW);
  bool isbuttonread = (buttonread == LOW);
  //Serial.println(isFloatOneUp);
  //Serial.println(isFloatTwoUp);
 
  if (isbuttonread) {
      digitalWrite(MotorHigh, HIGH);
      digitalWrite(MotorLow, LOW);
 }else{
      digitalWrite(MotorHigh, LOW);
      digitalWrite(MotorLow, LOW);
 }
   
  
  if (isFloatOneUp and isFloatTwoUp) {
    digitalWrite(redLED,  HIGH);
    digitalWrite(greenLED, LOW); 
  }
  else {
    digitalWrite(redLED,   LOW);
    //red light off
    digitalWrite(greenLED, HIGH);
    //green light on
  } 
}
//qwerty
