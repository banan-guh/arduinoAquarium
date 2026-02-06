
const int LEDPin = 14;

void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);
pinMode(LEDPin, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
digitalWrite(LEDPin, HIGH);
delay(1000);
digitalWrite(LEDPin, LOW);
delay(1000);
}
