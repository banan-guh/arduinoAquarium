

const int sensorthresholdtoptank = 2046;
const int Greentoptank = 3;
const int Redtoptank = 4;
const int floatone = 5;
const int floattwo = 6;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(Greentoptank, OUTPUT);
  pinMode(Redtoptank, OUTPUT);

 pinMode(floattwo, INPUT_PULLUP);
 pinMode(floatone, INPUT_PULLUP);
}

void loop() {
  // put your main code here, to run repeatedly:
  int levelone = digitalRead(floatone);
  int leveltwo = digitalRead(floattwo);
//  float waterlevelbottomtankone = analogRead(A1);

bool float1up = (digitalRead(levelone) == LOW);
bool float2up = (digitalRead(leveltwo)== LOW);

Serial.println(float1up);
Serial.println(float2up);


 // Serial.println(waterleveltoptanktwo);

 if (float2up or float1up) {
   digitalWrite(Redtoptank,LOW);
    digitalWrite(Greentoptank, HIGH);
  }else{
     digitalWrite(Redtoptank, HIGH);
    digitalWrite(Greentoptank, LOW); 
  } 
}
