#include <WiFi.h>
#include <esp_wifi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

// Old MAC Address: 6C:C8:40:89:A0:3C - DONT USE!!!
uint8_t newMACAddress[] = {0x6C, 0xC8, 0x40, 0x89, 0xA0, 0x3C};

const int floatTopUpPin = 25;
const int floatTopDownPin = 26;
const int floatBottomDownPin = 27;

const int RelayPin = 23;
const int debugLED = 21;

const String ssid = "SD23 IOT";
const String password = "l!ghtbox98"; 

int sensorPins[] = { floatTopUpPin, floatTopDownPin, floatBottomDownPin };
int strikeCounters[] = { 0, 0, 0 };
bool stableReadings[] = { false, false, false };

// --- FREE RTOS & LED STATES ---
QueueHandle_t requestQueue;

enum LedMode {
  LED_WIFI_CONNECTING,
  LED_WIFI_CONNECTED_SUCCESS,
  LED_NORMAL,
  LED_ERROR
};
volatile LedMode currentLedMode = LED_WIFI_CONNECTING;

void smartDelay(int ms) {
  LedMode startMode = currentLedMode; 
  
  // Break the delay down into tiny 25-millisecond checks
  for (int i = 0; i < ms; i += 25) {
    if (currentLedMode != startMode) {
      return; // The state changed! Abort the delay instantly.
    }
    vTaskDelay(pdMS_TO_TICKS(50)); 
  }
}

// ---------------- BACKGROUND LED TASK ----------------
void ledTask(void *pvParameters) {
  for(;;) {
    switch (currentLedMode) {
      case LED_WIFI_CONNECTING:
        // "Slow flash on top of fast flash" (Short, Short, Long pattern)
        digitalWrite(debugLED, HIGH); smartDelay(50);
        digitalWrite(debugLED, LOW);  smartDelay(100);
        digitalWrite(debugLED, HIGH); smartDelay(50);
        digitalWrite(debugLED, LOW);  smartDelay(100);
        digitalWrite(debugLED, HIGH); smartDelay(600); // The slow flash
        digitalWrite(debugLED, LOW);  smartDelay(400);
        break;
        
      case LED_WIFI_CONNECTED_SUCCESS:
        // 2 seconds solid to show successful connection, then move to normal
        digitalWrite(debugLED, HIGH);
        vTaskDelay(pdMS_TO_TICKS(2000));
        currentLedMode = LED_NORMAL; 
        break;
        
      case LED_NORMAL:
        // Brief blip every 10 seconds to prove the board hasn't frozen
        digitalWrite(debugLED, HIGH); vTaskDelay(pdMS_TO_TICKS(50));
        digitalWrite(debugLED, LOW);  smartDelay(9950);
        break;

      case LED_ERROR:
        // Frantic, rapid flashing
        digitalWrite(debugLED, HIGH); vTaskDelay(pdMS_TO_TICKS(50));
        digitalWrite(debugLED, LOW);  vTaskDelay(pdMS_TO_TICKS(450));
        break;
    }
  }
}

// ---------------- BACKGROUND HTTP TASK ----------------
void httpTask(void *pvParameters) {
  int statusToReport;
  for(;;) {
    // This waits indefinitely for an item in the queue.
    if (xQueueReceive(requestQueue, &statusToReport, portMAX_DELAY)) {
      
      // NEW: If we have a request, but WiFi isn't connected yet, 
      // wait patiently in the background until it is!
      while (WiFi.status() != WL_CONNECTED) {
        vTaskDelay(pdMS_TO_TICKS(500)); // Check again every half second
      }

      // WiFi is connected! Safe to send to Google.
      httpsRequest(statusToReport); 
    }
  }
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

void setup() {
  Serial.begin(115200);
  pinMode(RelayPin, OUTPUT);
  pinMode(debugLED, OUTPUT);
  
  pinMode(floatTopUpPin, INPUT_PULLUP);
  pinMode(floatTopDownPin, INPUT_PULLUP);
  pinMode(floatBottomDownPin, INPUT_PULLUP);

  // 1. Initialize Queue (Holds up to 10 HTTP requests)
  requestQueue = xQueueCreate(10, sizeof(int));

  // 2. Start Background Tasks
  xTaskCreate(ledTask, "LED_TASK", 2048, NULL, 1, NULL);
  xTaskCreate(httpTask, "HTTP_TASK", 8192, NULL, 1, NULL); // Larger stack for TLS/SSL

  delay(500);
  Serial.println("---------------------------------------------");
  
  WiFi.setSleep(false);
  WiFi.STA.begin();
  WiFi.setScanMethod(WIFI_FAST_SCAN);
  delay(500);

  Serial.println(WiFi.macAddress());
  WiFi.onEvent(WiFiEvent);

  Serial.println("Initiating WiFi connection in background...");
  WiFi.begin(ssid, password); // Non-blocking connection

  for (int i = 0; i < 10; i++) {
    takeStableReadings();
  }
}

unsigned long OneDay = 86400000; // ms
unsigned long lastOKSmall = 0; 
unsigned long lastOKBig = 0; 

bool firstOKSmall = true;
bool firstOKBig = true;
bool firstError = true;

bool triggered = false;

void loop() {
  manageWiFi();
  if (!triggered) takeStableReadings();

  // Master controller to check if anything wrong
  // Note: sumTingWongCounter completely scrapped. Instantly reacts.
  bool sumTingWong = true;
  if (!triggered) sumTingWong = !stableReadings[0] || !stableReadings[1] || !stableReadings[2];
  
  // Update the LED background task based on current state
  if (WiFi.status() != WL_CONNECTED) {
    currentLedMode = LED_WIFI_CONNECTING;
  }
  else if (sumTingWong) {
    currentLedMode = LED_ERROR;
  }
  else if (currentLedMode == LED_WIFI_CONNECTING) {
    // We just connected to wifi! Show success state.
    currentLedMode = LED_WIFI_CONNECTED_SUCCESS; 
  } 
  else if (currentLedMode != LED_WIFI_CONNECTED_SUCCESS) {
    // If not showing success sequence, just default to normal
    currentLedMode = LED_NORMAL;
  }

  // Handle Relay and Web Requests
  if (sumTingWong) {
    triggered = true;
    digitalWrite(RelayPin, HIGH);
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
  
  delay(200); 
}

// Push to the queue instantly
void enqueueRequest(int status) {
  if (xQueueSend(requestQueue, &status, 0) != pdPASS) {
    Serial.println("Queue full! Request dropped.");
  }
}

void requestOK() {
  unsigned long currentTime = millis();
  if (firstOKBig || currentTime - lastOKBig >= OneDay * 7) {
    enqueueRequest(200); 
    lastOKBig = currentTime;
    lastOKSmall = currentTime;
    firstOKBig = false;
    firstOKSmall = false; 
  }
  else if (firstOKSmall || currentTime - lastOKSmall >= OneDay / 24) {
    enqueueRequest(201); 
    lastOKSmall = currentTime;
    firstOKSmall = false;
  }
}

unsigned long lastError = 0; 
void requestError() {
  unsigned long currentTime = millis();
  if (firstError || currentTime - lastError >= OneDay / 48) { 
    Serial.println("HELP!!!! Error state triggered, sending to Google...");
    enqueueRequest(-1); 
    lastError = currentTime;
    firstError = false; 
  }
}

void takeStableReadings() {
  for (int x = 0; x < 3; x++) { // take 3 readings
    for (int i = 0; i < 3; i++) { // take 1 reading per sensor
      takeStableReading(i);
    }
    delay(10); 
  }
}

void takeStableReading(int index) {
  bool reading = digitalRead(sensorPins[index]);
  if (reading == true) strikeCounters[index]++;
  else strikeCounters[index]--;
  
  int threshold = 10;
  int clampMax = 15; 
  
  if (strikeCounters[index] > threshold) {
    stableReadings[index] = true;
  }
  else stableReadings[index] = false;

  strikeCounters[index] = max(0, min(clampMax, strikeCounters[index]));
}

void manageWiFi() {
  const long TIME = 30000;
  static unsigned long lastRetry = millis();
  static unsigned long interval = TIME;

  if (WiFi.status() == WL_CONNECTED) {
    interval = TIME;
    return;
  }
  
  if (millis() - lastRetry >= interval) {
    Serial.println("WiFi not connected. Attempting reconnect...");
    WiFi.reconnect(); 
    
    lastRetry = millis();
    interval = min(interval * 2, 3600000UL); // Max 1 hour
  }
}

void httpsRequest(int status) {
  WiFiClientSecure secureClient; 
  secureClient.setInsecure();

  String url = "https://script.google.com/macros/s/AKfycbwdZ939IvwVngNw_QaKxRxyCrZRkLUvCy8W2hjJjwRwRUBWiH7Kasa8LhqxrhOosQZg/exec";
  url += "?status=" + String(status);

  HTTPClient http;

  Serial.print("Connecting to server for status: ");
  Serial.println(status);
  
  if (http.begin(secureClient, url)) {
    http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS); 
    
    int httpCode = http.GET();
    Serial.print("HTTP Code: ");
    Serial.println(httpCode);

    if (httpCode > 0) {
      Serial.println("Success");
    } else {
      Serial.println("HTTP GET failed");
    }
    
    http.end();
  } else {
    Serial.println("Failed to connect to server.");
  }
}