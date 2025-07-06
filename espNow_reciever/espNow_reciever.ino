#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

// LCD Configuration (PCF8574 I2C address typically 0x27)
LiquidCrystal_PCF8574 lcd(0x27); // Adjust address if needed

// Data structure for received information
typedef struct struct_message {
  // GPS Data
  float latitude;
  float longitude;
  float altitude;
  int satellites;

  // Environmental Data
  float temperature;
  float humidity;

  // Health Data
  int heart_beat;

  // System Data
  int battery_level;

  // Time Data
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;

  // Motion Data (New)
  float gyroX;    // X-axis rotation (degrees/second)
  float gyroY;    // Y-axis rotation
  float gyroZ;    // Z-axis rotation
  float motion;   // Composite motion value
} struct_message;

struct_message sensorData;
bool newDataReceived = false;
unsigned long lastDataTime = 0;

// Display management
enum DisplayMode { REAL_TIME, PREDICTIONS, MOTION };
DisplayMode currentMode = REAL_TIME;
unsigned long lastDisplayToggle = 0;
const unsigned long DISPLAY_TOGGLE_INTERVAL = 5000; // 5 seconds

// Initialize ESP-NOW
void initESP_NOW() {
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    lcd.clear();
    lcd.print("ESP-NOW Init Fail");
    while (1) delay(100);
  }

  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("ESP-NOW Initialized");
}

// Updated Callback when data is received
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  memcpy(&sensorData, incomingData, sizeof(sensorData));
  newDataReceived = true;
  lastDataTime = millis();
  Serial.println("New data received");
}

// Display real-time sensor data (Updated)
void displayRealTimeData() {
  lcd.clear();

  // Line 1: GPS Coordinates
  lcd.setCursor(0, 0);
  lcd.print("Lat:");
  lcd.print(sensorData.latitude, 4);
  lcd.print(" Lon:");
  lcd.print(sensorData.longitude, 4);

  // Line 2: Environmental Data
  lcd.setCursor(0, 1);
  lcd.print("Temp:");
  lcd.print(sensorData.temperature, 1);
  lcd.print("C Hum:");
  lcd.print(sensorData.humidity, 0);
  lcd.print("%");

  // Line 3: Health Data
  lcd.setCursor(0, 2);
  lcd.print("Pulse:");
  lcd.print(sensorData.heart_beat);
  lcd.print(" BPM Bat:");
  lcd.print(sensorData.battery_level);
  lcd.print("%");

  // Line 4: Last Update Time
  lcd.setCursor(0, 3);
  lcd.print("Updated: ");
  lcd.print(sensorData.hour);
  lcd.print(":");
  if (sensorData.minute < 10) lcd.print("0");
  lcd.print(sensorData.minute);
}

// Display health predictions and alerts (Updated)
void displayPredictions() {
  lcd.clear();

  // Determine statuses
  String pulseStatus = (sensorData.heart_beat > 100) ? "HIGH" :
                       (sensorData.heart_beat < 60) ? "LOW" : "NORMAL";

  String tempStatus = (sensorData.temperature > 37.5) ? "HIGH" :
                      (sensorData.temperature < 36.0) ? "LOW" : "NORMAL";

  String batteryStatus = (sensorData.battery_level < 20) ? "LOW!" : "OK";

  // Line 1: Header
  lcd.setCursor(0, 0);
  lcd.print("HEALTH ANALYSIS");

  // Line 2: Pulse Status
  lcd.setCursor(0, 1);
  lcd.print("Pulse: ");
  lcd.print(pulseStatus);

  // Line 3: Temperature Status
  lcd.setCursor(0, 2);
  lcd.print("Temp: ");
  lcd.print(tempStatus);

  // Line 4: Battery Status
  lcd.setCursor(0, 3);
  lcd.print("Battery: ");
  lcd.print(batteryStatus);
}

// New function to display motion data
void displayMotionData() {
  lcd.clear();

  // Line 1: Gyro X and Y
  lcd.setCursor(0, 0);
  lcd.print("Gyro X:");
  lcd.print(sensorData.gyroX, 1);
  lcd.print(" Y:");
  lcd.print(sensorData.gyroY, 1);

  // Line 2: Gyro Z and Motion
  lcd.setCursor(0, 1);
  lcd.print("Gyro Z:");
  lcd.print(sensorData.gyroZ, 1);
  lcd.print(" Mot:");
  lcd.print(sensorData.motion, 1);

  // Line 3-4: Motion visualization
  lcd.setCursor(0, 2);
  lcd.print("Motion Level:");

  lcd.setCursor(0, 3);
  int motionLevel = map(constrain(abs(sensorData.motion), 0, 500), 0, 500, 0, 20);
  for (int i = 0; i < motionLevel; i++) {
    lcd.print(">");
  }
}

// Display waiting for data message
void displayWaitingForData() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting for data...");

  lcd.setCursor(0, 2);
  lcd.print("Last received: ");
  if (lastDataTime == 0) {
    lcd.print("Never");
  } else {
    unsigned long secondsAgo = (millis() - lastDataTime) / 1000;
    lcd.print(secondsAgo);
    lcd.print("s ago");
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(); // Initialize I2C

  // Initialize LCD
  lcd.begin(20, 4); // 20 columns, 4 rows
  lcd.setBacklight(255); // Full backlight
  lcd.clear();
  lcd.home();
  lcd.print("Initializing...");

  // Initialize ESP-NOW
  initESP_NOW();

  lcd.clear();
  lcd.print("System Ready");
  delay(1000);
}

void loop() {
  // Toggle display mode periodically (Updated for 3 modes)
  if (millis() - lastDisplayToggle >= DISPLAY_TOGGLE_INTERVAL) {
    currentMode = (DisplayMode)((currentMode + 1) % 3); // Cycles through 0,1,2
    lastDisplayToggle = millis();
  }

  // Display appropriate screen (Updated)
  if (!newDataReceived && lastDataTime == 0) {
    displayWaitingForData();
  } else {
    switch (currentMode) {
      case REAL_TIME:
        displayRealTimeData();
        break;
      case PREDICTIONS:
        displayPredictions();
        break;
      case MOTION:
        displayMotionData();
        break;
    }
  }

  newDataReceived = false;
  delay(100);
}
