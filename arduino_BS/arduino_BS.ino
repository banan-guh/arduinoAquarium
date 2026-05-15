#include <WiFi.h>
#include <esp_wifi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// Old MAC Address: 6C:C8:40:89:A0:3C
uint8_t newMACAddress[] = {0x6C, 0xC8, 0x40, 0x89, 0xA0, 0x3C};


const int floatTopUpPin = 25;
const int floatTopDownPin = 26;
const int floatBottomDownPin = 27;

//const int MotorHighPin = 32;
//const int MotorLowPin = 33;
const int RelayPin = 23;
const int debugLED = 21;

//const String ssid = "SD23 IOT";
//const String password = "l!ghtbox98"; // use iot instead of guest, no need for mac spoofing
const String ssid = "yuh 9001";
const String password = "ErmDying";



// for takereading func below
int sensorPins[] = { floatTopUpPin, floatTopDownPin, floatBottomDownPin };
int strikeCounters[] = { 0, 0, 0 };
bool stableReadings[] = { false, false, false };

void flashLED(int delayTime = 50) {
  digitalWrite(debugLED, HIGH);
  delay(delayTime);
  digitalWrite(debugLED, LOW);
}

void WiFiEvent(arduino_event_id_t event) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_START:
      Serial.println("WiFi client started");
      break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      Serial.println("Connected to access point");
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.print("IP obtained: ");
      Serial.println(WiFi.localIP());
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("Connection failed or lost");
      break;
  }
}

IPAddress staticIP(10, 132, 144, 150);// Desired Static IP
IPAddress gateway(10, 132, 144, 1);   // Router IP
IPAddress subnet(255, 255, 240, 0);   // Subnet Mask
IPAddress primaryDNS(8, 8, 8, 8);     // Optional DNS
IPAddress secondaryDNS(0, 0, 0, 0);   // Optional DNS

void setup() {
  Serial.begin(115200);
  pinMode(RelayPin, OUTPUT);
  pinMode(debugLED, OUTPUT);
  
  pinMode(floatTopUpPin, INPUT_PULLUP);
  pinMode(floatTopDownPin, INPUT_PULLUP);
  pinMode(floatBottomDownPin, INPUT_PULLUP);

  //WiFi.config(staticIP, gateway, subnet, primaryDNS, secondaryDNS);

  delay(500);

  Serial.println("---------------------------------------------");

  flashLED(500);
  WiFi.setSleep(false);
  //WiFi.mode(WIFI_STA);
  WiFi.STA.begin();
  
  delay(500);

  //esp_err_t err = esp_wifi_set_mac(WIFI_IF_STA, &newMACAddress[0]);
  //if (err == ESP_OK) {
    //Serial.println("\nSuccess changing Mac Address");
  //}
  //else { Serial.println("crap"); }
  Serial.println(WiFi.macAddress());
  //Serial.print("Connecting.");
  WiFi.onEvent(WiFiEvent);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    flashLED(500);
    delay(500);
    
  }
  Serial.println(" Connected!");
  for (int i = 0; i < 10; i++) {
    takeStableReadings();
  }

  Serial.println(WiFi.gatewayIP());
  delay(1000);
}

// from requestOK, have to put here for loop to recognize
unsigned long OneDay = 86400000; // ms
unsigned long lastOKSmall = 0; // initialize at 0
unsigned long lastOKBig = 0; // initialize at 0

bool firstOKSmall = true;
bool firstOKBig = true;
bool firstError = true;


void loop() {
  // top = top tank, bottom = bottom tank
  // up = sense when water goes up, down = sense when water goes down

  manageWiFi();

  takeStableReadings();

  // stablereadings: 0 = top up (27), 1 = bottom up (26), 2 = bottom down (25)
  //Serial.println("Readings #1, #2, #3: " + String(stableReadings[0]) + ", " + String(stableReadings[1]) + ", " + String(stableReadings[2]));
  //Serial.println("Readings #1, #2, #3: " + String(digitalRead(floatTopUpPin)) + ", " + String(digitalRead(floatTopDownPin)) + ", " + String(digitalRead(floatBottomDownPin)));
  //Serial.println("strike:" + String(strikeCounters[0]));

  bool sumTingWong = stableReadings[0] || !stableReadings[1] || !stableReadings[2];
  // master controller to check if anything wrong, led control is after
  if (sumTingWong) {
    digitalWrite(RelayPin, HIGH);
    //digitalWrite(MotorHigh, HIGH);
    //digitalWrite(MotorLow, LOW);

    Serial.println("HELP!!!!");
    flashLED(50);
    delay(100);
    flashLED(50);
    delay(500);
    requestError();
  }
  else {
    if (!firstError) {
      firstOKBig = true;
      Serial.println("Forcing 200 status");
    }
    digitalWrite(RelayPin, LOW);
    firstError = true;
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

  delay(100); // increase delay later
}


void requestOK() {
  unsigned long currentTime = millis();
  if (firstOKBig || currentTime - lastOKBig >= OneDay * 2) {
    httpsRequest(200);
    lastOKBig = millis();
    lastOKSmall = millis();
    firstOKBig = false;
    firstOKSmall = false; // reset small one too
  }
  else if (firstOKSmall || currentTime - lastOKSmall >= OneDay / 24) {
    httpsRequest(201);
    lastOKSmall = millis();
    firstOKSmall = false;
  }
  // else dont do anything
}


unsigned long lastError = 0; // last error timestamp
void requestError() {
  unsigned long currentTime = millis();
  if (firstError || currentTime - lastError >= OneDay / 48) { // screams every 30 mins
    httpsRequest(-1);
    Serial.println("sent!");
    lastError = millis();
    firstError = false; // Stop it from firing continuously
  }
  else {Serial.println("blocked - timeout");}
  // else dont do anything
}

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
  static unsigned long lastRetry = 0;
  static unsigned long interval = 10000; // Start with 10 seconds

  if (WiFi.status() == WL_CONNECTED) {
    interval = 60000; // Reset delay when connected
    return;
  }
  
  if (lastRetry == 0 || millis() - lastRetry >= interval) {
    Serial.println("Reconnecting...");
    WiFi.reconnect(); // Much safer/cleaner than disconnect() + begin()
    
    lastRetry = millis();
    interval = min(interval * 2, 3600000UL); // Double it, max 1 hour
  }
}


// sends an https request to the aquariumScream google apps script to send emails, update website, etc.
// find aquar login on awlms google doccy.
void httpsRequest(int status) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected, skipping request.");
    return;
  }

  // 1. Declare the client LOCALLY so it resets every time
  WiFiClientSecure secureClient; 
  secureClient.setInsecure();
  // secureClient.setTimeout(15000); // Optional, but usually good to keep

  String url = "https://script.google.com/macros/s/AKfycbydJjaLl7ykkEbpGcqZB-bQGoa4jnrq9skWRvU00BoBAst9lSNDwM60rFoqrgd__ClJ/exec";
  url += "?status=" + String(status);

  HTTPClient http;

  Serial.println("Connecting to server...");
  if (http.begin(secureClient, url)) {
    
    // 2. FORCE following redirects to ensure variables aren't dropped
    http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS); 
    
    int httpCode = http.GET();
    Serial.print("HTTP Code: ");
    Serial.println(httpCode);

    if (httpCode > 0) {
      // 3. Print the payload to confirm Google didn't send a Login page
      String payload = http.getString();
      Serial.println("Payload from Google:");
      Serial.println(payload);
    } else {
      Serial.printf("HTTP GET failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    
    http.end();
  } else {
    Serial.println("Failed to connect to server.");
  }
}