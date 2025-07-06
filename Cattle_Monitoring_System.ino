#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include <TinyGPSPlus.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <DHT.h>
#include <esp_now.h>
#include <WiFi.h>
// -------------------- Network Credentials --------------------
const char apn[] = "internet";
const char gprsUser[] = "";
const char gprsPass[] = "";

const char *UBIDOTS_TOKEN = "BBUS-2M3ALmj3m4Qg54Ao9SBRz5jejg0odI";
const char *DEVICE_LABEL = "tracking_device";
const char *MQTT_BROKER = "industrial.api.ubidots.com";
const int MQTT_PORT = 1883;

// -------------------- Pin Definitions --------------------
#define SerialMon Serial
#define SerialAT Serial1
#define TINY_GSM_DEBUG SerialMon
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
PubSubClient mqtt(client);

#define modem_Switch       4
#define MODEM_TX          16
#define MODEM_RX          17
#define gps_RXD           27
#define gps_TXD           26
#define gps_Switch        14
#define heartBeat         13
#define batteryPin        34
#define DHTPIN            14
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

Adafruit_MPU6050 mpu;
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0x88, 0x13, 0xbf, 0x24, 0xe4, 0xec};

// Define a struct to hold the data
typedef struct struct_message {
  // Your existing data
  float latitude;
  float longitude;
  float altitude;
  int satellites;
  float temperature;
  float humidity;
  int heart_beat;
  int battery_level;
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  float gyroX;  // X-axis rotation (degrees/second)
  float gyroY;  // Y-axis rotation
  float gyroZ;  // Z-axis rotation

} struct_message;

// Create a struct_message called myData
struct_message myData;

esp_now_peer_info_t peerInfo;


unsigned long previousMillis = 0;
unsigned long interval = 60000;

// -------------------- Setup Function --------------------
void setup() {
  setUp_functions();
}

void loop() {
  if (!mqtt.connected()) {
    SerialMon.println("MQTT not connected. Attempting to reconnect...");
    connectToMQTT();
  }
  publish_all_sensor_data();
  mqtt.loop();

}
