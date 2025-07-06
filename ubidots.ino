// -------------------- MQTT Connection --------------------
void connectToMQTT() {
  SerialMon.println("Connecting to MQTT broker...");
  while (!mqtt.connected()) {
    if (mqtt.connect(DEVICE_LABEL, UBIDOTS_TOKEN, "")) {
      SerialMon.println("Connected to MQTT broker");
    } else {
      SerialMon.print("Failed to connect to MQTT, rc=");
      SerialMon.print(mqtt.state());
      SerialMon.println(". Retrying in 10 seconds...");
      delay(10000); // Wait 10 seconds before retrying
    }
  }
}

// -------------------- Publish Sensor Data --------------------
void publish_sensor_data(const char *variable, String payload) {
  char msgBuffer[512]; // Buffer to hold the payload
  payload.toCharArray(msgBuffer, 512); // Convert String to char array
  SerialMon.println("Publishing: " + payload);

  if (mqtt.publish(variable, msgBuffer)) {
    SerialMon.println("Data published successfully");
  } else {
    SerialMon.println("Failed to publish data");
  }
}

// -------------------- Publish All Sensor Data --------------------
void publish_all_sensor_data() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {

    // Ensure MQTT is connected
    if (!mqtt.connected()) {
      connectToMQTT();
    }

    // Reading sensor data
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    float heatIndex = dht.computeHeatIndex(temperature, humidity, false);

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    float gyroTemperature = temp.temperature;
    while (gpsSerial.available()) {
      char c = gpsSerial.read();
      gps.encode(c);   // Parse the GPS data
    }
    float latitude = gps.location.lat();
    float longitude = gps.location.lng();
    float altitude = gps.altitude.meters();
    int satellites = gps.satellites.value();
    int heart_beat = analogRead(heartBeat);
    int battery_level = analogRead(batteryPin);
    heart_beat = map(heart_beat, 0, 2430, 0, 300);
    battery_level = map(battery_level, 0 , 2430, 0, 100);

    // Set values to send
    myData.latitude = latitude;
    myData.longitude = longitude;
    myData.altitude = altitude;
    myData.satellites = satellites;
    myData.temperature = temperature;
    myData.humidity = humidity;
    myData.heart_beat = heart_beat;
    myData.battery_level = battery_level;

    myData.gyroX = g.gyro.x;
    myData.gyroY = g.gyro.y;
    myData.gyroZ = g.gyro.z;

    timeFetch();

    // Send the data using ESP-NOW
    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    // Debug output
    Serial.println("Data sent via ESP-NOW:");
    Serial.print("Latitude: "); Serial.println(myData.latitude, 6);
    Serial.print("Longitude: "); Serial.println(myData.longitude, 6);
    Serial.print("Altitude: "); Serial.println(myData.altitude);
    Serial.print("Satellites: "); Serial.println(myData.satellites);
    Serial.print("Temperature: "); Serial.println(myData.temperature);
    Serial.print("Humidity: "); Serial.println(myData.humidity);
    Serial.print("Heart Beat: "); Serial.println(myData.heart_beat);
    Serial.print("Battery Level: "); Serial.println(myData.battery_level);
    espNowSent();

    // Construct the base topic string
    String baseTopic = "/v1.6/devices/" + String(DEVICE_LABEL);

    // Publish temperature data
    String temperaturePayload = "{\"value\": " + String(temperature) + "}";
    publish_sensor_data((baseTopic + "/body_temperature").c_str(), temperaturePayload);

    // Publish humidity data
    String humidityPayload = "{\"value\": " + String(humidity) + "}";
    publish_sensor_data((baseTopic + "/humidity").c_str(), humidityPayload);

    // Publish heat index data
    String heatIndexPayload = "{\"value\": " + String(heatIndex) + "}";
    publish_sensor_data((baseTopic + "/heat_index").c_str(), heatIndexPayload);

    // Publish gyro temperature data
    String gyroTemperaturePayload = "{\"value\": " + String(gyroTemperature) + "}";
    publish_sensor_data((baseTopic + "/temperature").c_str(), gyroTemperaturePayload);

    // Publish altitude data
    String altitudePayload = "{\"value\": " + String(altitude) + "}";
    publish_sensor_data((baseTopic + "/altitude").c_str(), altitudePayload);

    // Publish heart beat data
    String heartBeatPayload = "{\"value\": " + String(heart_beat) + "}";
    publish_sensor_data((baseTopic + "/heart_beat").c_str(), heartBeatPayload);

    // Publish battery data
    String batteryPayload = "{\"value\": " + String(battery_level) + "}";
    publish_sensor_data((baseTopic + "/battery_level").c_str(), batteryPayload);

    // Publish acceleration data
    String accelerationXPayload = "{\"value\": " + String(a.acceleration.x) + "}";
    publish_sensor_data((baseTopic + "/acceleration_x").c_str(), accelerationXPayload);

    String accelerationYPayload = "{\"value\": " + String(a.acceleration.y) + "}";
    publish_sensor_data((baseTopic + "/acceleration_y").c_str(), accelerationYPayload);

    String accelerationZPayload = "{\"value\": " + String(a.acceleration.z) + "}";
    publish_sensor_data((baseTopic + "/acceleration_z").c_str(), accelerationZPayload);

    // Publish rotation data
    String rotationXPayload = "{\"value\": " + String(g.gyro.x) + "}";
    publish_sensor_data((baseTopic + "/rotation_x").c_str(), rotationXPayload);

    String rotationYPayload = "{\"value\": " + String(g.gyro.y) + "}";
    publish_sensor_data((baseTopic + "/rotation_y").c_str(), rotationYPayload);

    String rotationZPayload = "{\"value\": " + String(g.gyro.z) + "}";
    publish_sensor_data((baseTopic + "/rotation_z").c_str(), rotationZPayload);

    // Publish GPS position data
    if (gps.location.isValid()) {
      // Create a JSON payload for position (latitude and longitude only)
      String positionPayload = "{\"latitude\": " + String(latitude, 6) + ", "
                               "\"longitude\": " + String(longitude, 6) + "}";
      SerialMon.println("Position Payload: " + positionPayload); // Debug output
      publish_sensor_data((baseTopic + "/position").c_str(), positionPayload);

    } else {
      SerialMon.println("Invalid GPS data. Skipping position and additional GPS data publish.");
    }

    previousMillis = currentMillis;
  }
}
