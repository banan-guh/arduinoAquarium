#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>


//const int greenLEDTopPin = 2;
//const int redLEDTopPin = 23;
//const int greenLEDBottomPin = 13;
//const int redLEDBottomPin = 27;

const int floatTopUpPin = 18;
const int floatTopDownPin = 19;
const int floatBottomDownPin = 21;

//const int MotorHighPin = 18;
//const int MotorLowPin = 19;
const int RelayPin = 4;

const String ssid = "SD23 IOT";
const String password = "l!ghtbox98"; // use iot instead of guest, no need for mac spoofing


// for takereading func below
int sensorPins[] = { floatTopUpPin, floatTopDownPin, floatBottomDownPin };
int strikeCounters[] = { 0, 0, 0 };
bool stableReadings[] = { false, false, false };


void setup() {
  Serial.begin(115200);
  //pinMode(greenLEDTopPin, OUTPUT);
  //pinMode(redLEDTopPin, OUTPUT);
  //pinMode(greenLEDBottomPin, OUTPUT);
  //pinMode(redLEDBottomPin, OUTPUT);

  //pinMode(MotorHighPin, OUTPUT);
  //pinMode(MotorLowPin, OUTPUT);

  pinMode(RelayPin, OUTPUT);
  
  pinMode(floatTopUpPin, INPUT_PULLUP);
  pinMode(floatTopDownPin, INPUT_PULLUP);
  pinMode(floatBottomDownPin, INPUT_PULLUP);

  WiFi.begin(ssid, password);
  Serial.print("Connecting.");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println(" Connected!");
  delay(1000);
}


void loop() {
  // top = top tank, bottom = bottom tank
  // up = sense when water goes up, down = sense when water goes down

  manageWiFi();

  takeStableReadings();

  // stablereadings: 0 = top up, 1 = top down, 2 = bottom down
  //Serial.println("Readings #1, #2, #3: " + String(stableReadings[0]) + ", " + String(stableReadings[1]) + ", " + String(stableReadings[2]));
  //Serial.println("Readings #1, #2, #3: " + String(digitalRead(floatTopUpPin)) + ", " + String(digitalRead(floatTopDownPin)) + ", " + String(digitalRead(floatBottomDownPin)));
  //Serial.println("strike:" + String(strikeCounters[0]));

  bool sumTingWong = stableReadings[0]; //|| !stableReadings[1] || !stableReadings[2];
  // master controller to check if anything wrong, led control is after
  if (sumTingWong) {
    digitalWrite(RelayPin, HIGH);
    //digitalWrite(MotorHigh, HIGH);
    //digitalWrite(MotorLow, LOW);

    Serial.println("HELP!!!!");
    requestError();
  }
  else {
    //digitalWrite(MotorHigh, LOW);
    //digitalWrite(MotorLow, LOW);
    digitalWrite(RelayPin, LOW);
    requestOK();
  }
  
  // ---------- led control -----------
  // Top LED control
  if (stableReadings[0] || !stableReadings[1]) {
    //digitalWrite(redLEDTopPin,  HIGH);
    //digitalWrite(greenLEDTopPin, LOW); 
  } 
  else {
    //digitalWrite(redLEDTopPin,   LOW);
    //digitalWrite(greenLEDTopPin, HIGH);
  }

  // Bottom LED control
  if (!stableReadings[2]) {
    //digitalWrite(redLEDBottomPin, HIGH);
    //digitalWrite(greenLEDBottomPin, LOW);
  }
  else {
    //digitalWrite(redLEDBottomPin, LOW);
    //digitalWrite(greenLEDBottomPin, HIGH);
  }

  delay(1000); // increase delay later
}


unsigned long OneDay = 86400000; // ms
unsigned long lastOKSmall = millis(); // timestamp: small = update website, no email
unsigned long lastOKBig = millis(); // timestamp: big = both website and email
void requestOK() {
  unsigned long currentTime = millis();
  if (currentTime - lastOKBig >= OneDay * 2) {
    httpsRequest(200);
    lastOKBig = millis();
    lastOKSmall = millis();
  }
  if (currentTime - lastOKSmall >= OneDay / 6) {
    httpsRequest(201);
    lastOKSmall = millis();
  }
  // else dont do anything
}


unsigned long lastError = 0; // last error timestamp
void requestError() {
  unsigned long currentTime = millis();
  if (currentTime - lastError >= OneDay / 48) { // screams every 30 mins
    httpsRequest(-1);
    lastError = millis();
  }
  // else dont do anything
}


// vars used to be here

// wrapper for takestablereading
void takeStableReadings() {
  for (int x = 0; x < 1; x++) { // take reading x times (using 1 because its too sensitive otherwise)
    for (int i = 0; i < 3; i++) {
      takeStableReading(i);
    }
    delay(10); // take each reading 0.01 seconds apart
  }
}


// this is different from the other functions, it uses index instead of pin # and uses that index
// to do array stuff.
// this makes sure false positives are less easy, it makes the program do a bunch of 'hits in a row' before it
// can activate the actual reading.
void takeStableReading(int index) {
  bool reading = digitalRead(sensorPins[index]);
  if (reading == true) strikeCounters[index]++;
  else strikeCounters[index]--;
  int threshold = 3; // arbitrary number
  int clampMax = 5; // upper bound of strikecounters`
  // sets stablereadings to true if it hits threshold
  if (strikeCounters[index] > threshold) {
    stableReadings[index] = true;
  }
  else stableReadings[index] = false;

  strikeCounters[index] = max(0, min(clampMax, strikeCounters[index]));
}


unsigned long previousMillis = 0;
unsigned long reconnectInterval = 1000; // Start with 1 second
const unsigned long maxInterval = 300000; // Max 5 minutes (300,000 ms)


void manageWiFi() {
  static unsigned long nextRetry = 0;
  static unsigned long interval = 5000; // Start with 5 seconds

  if (WiFi.status() == WL_CONNECTED) {
    interval = 5000; // Reset delay when connected
    return;
  }

  if (millis() > nextRetry) {
    Serial.println("Reconnecting...");
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    
    nextRetry = millis() + interval;
    interval = min(interval * 2, 3600000UL); // Double it, max 1 hour
  }
}


// sends an https request to the aquariumScream google apps script to send emails, update website, etc.
// find aquar login on awlms google doccy.
void httpsRequest(int status) {
  if (WiFi.status() != WL_CONNECTED) return;
  // 200 status = OK, -1 status = PANIC
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
