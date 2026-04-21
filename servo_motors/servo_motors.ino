
 #include <Servo.h>
int servopin1 = 6;
int servopin2 = 7;

Servo Servo1;
Servo Servo2;


void setup() {
  // put your setup code here, to run once:
   Serial.begin(9600);
  Servo1.attach(servopin1);
  Servo2.attach(servopin2);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  Servo1.write(180);
  delay(4000);
  Servo1.write(90);
  Serial.println("Servo1 is turning");
  delay(100);
  Servo2.write(180);
  delay(4000);
  Serial.println("Servo2 is turning");
  Servo2.write(90);
}
