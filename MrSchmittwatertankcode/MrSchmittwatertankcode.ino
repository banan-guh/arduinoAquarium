
const int greenLED = 3;
const int redLED =   4;
const int floatOne = 5;
const int floatTwo = 6;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(greenLED, OUTPUT);
  pinMode(redLED,   OUTPUT);
  
  pinMode(floatTwo, INPUT_PULLUP);
  pinMode(floatOne, INPUT_PULLUP);
}

void loop() {
  // put your main code here, to run repeatedly:
  int readOne = digitalRead(floatOne);
  int readTwo = digitalRead(floatTwo);

  bool isFloatOneUp = (readOne == LOW);
  bool isFloatTwoUp = (readTwo == LOW);
  
  //Serial.println(isFloatOneUp);
  //Serial.println(isFloatTwoUp);

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
