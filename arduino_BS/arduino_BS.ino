#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

const int LEDPin = 27;
const int sensorOnePin = 25;

const String ssid = "SD23 IOT";
const String password = "????";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(sensorOnePin, INPUT_PULLUP);
  pinMode(LEDPin, OUTPUT);

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
  // put your main code here, to run repeatedly:
  digitalWrite(LEDPin, HIGH);
  //delay(10);
  //digitalWrite(LEDPin, LOW);
  //delay(10);
  //httpsRequest(200);
  int isFloatOneActivated = analogRead(sensorOnePin);
  Serial.println(isFloatOneActivated);
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

    //Serial.println(httpCode);
    http.end();
  }
}
