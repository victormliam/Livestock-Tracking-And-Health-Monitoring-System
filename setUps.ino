// -------------------- Setup Functions --------------------
void setUp_functions() {
  SerialMon.begin(115200);
  SerialAT.begin(9600, SERIAL_8N1, MODEM_RX, MODEM_TX); // Initialize SIM800L
  delay(1000);
  pinMode(modem_Switch, OUTPUT);
  pinMode(heartBeat, INPUT);
  pinMode(batteryPin, INPUT);
  digitalWrite(modem_Switch, HIGH);

  delay(1000);
  gpsSerial.begin(9600, SERIAL_8N1, gps_RXD, gps_TXD);
  connectToGPRS();
  gyroSet();  dht.begin();
  espNowSet();
  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
  connectToMQTT();
}
