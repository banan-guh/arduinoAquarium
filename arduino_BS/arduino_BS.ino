
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

const int LEDPin = 14;
const String ssid = "SD23 Guest";
const String password = "aquarium";

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
  httpsRequest(200);
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
