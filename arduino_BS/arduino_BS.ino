#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

const int greenLED = 32;
const int redLED =   27;
const int floatOne = 25;
const int floatTwo = 34;
const int MotorHigh = 23;
const int MotorLow = 22;
const int Button = 13;
const int Relay = 5;

const String ssid = "SD23 IOT";
const String password = "????";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(greenLED, OUTPUT);
  pinMode(redLED,   OUTPUT);
  //pinMode(MotorHigh, OUTPUT);
  //pinMode(MotorLow, OUTPUT);
  pinMode(Relay, OUTPUT);
  
  pinMode(floatTwo, INPUT_PULLUP);
  pinMode(floatOne, INPUT_PULLUP);
  pinMode(Button, INPUT_PULLUP);

  WiFi.begin(ssid, password);

  Serial.print("Connecting...");
  //while (WiFi.status() != WL_CONNECTED) {
    //delay(500);
    //Serial.print(".");
  //}
  Serial.println(" Connected!");
  delay(1000);
}

void loop() {
  //httpsRequest(200);
  int readOne = digitalRead(floatOne);
  int readTwo = digitalRead(floatTwo);
  int ButtonRead = digitalRead(Button);
  bool isFloatOneUp = (readOne == LOW);
  bool isFloatTwoUp = (readTwo == LOW);
  bool isButtonOn = (ButtonRead == LOW);
  //Serial.println(isFloatOneUp);
  //Serial.println(isFloatTwoUp);
  Serial.println(isButtonOn);
  if (isButtonOn) {
    digitalWrite(Relay, HIGH);
    //digitalWrite(MotorHigh, HIGH);
    //digitalWrite(MotorLow, LOW);
    Serial.println("guh uuh");
  }
  else {
    //digitalWrite(MotorHigh, LOW);
    //digitalWrite(MotorLow, LOW);
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

void httpsRequest(int status) {
  String url = "https://script.google.com/macros/s/AKfycbxCv_mbnUbU8EKLbrP3T5WFFZIns34vBTYhAx_b1ZEHG8KxVdpnt7GYZVgGXJWoTD58/exec";
  url += "?status=" + String(status);

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;

  if(http.begin(client, url)) {
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET();

    Serial.println(httpCode);
    http.end();
  }
}
