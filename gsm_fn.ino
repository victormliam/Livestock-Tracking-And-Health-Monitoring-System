void connectToGPRS() {

//  SerialMon.println("Restarting modem...");
//  if (!modem.restart()) {
//    SerialMon.println("Failed to restart modem!");
//    return;
//  }  SerialMon.println("Modem initialized!");

  delay(1000);

  int retryCount = 0;
  const int maxRetries = 5;
  while (retryCount < maxRetries) {
    SerialMon.print("Waiting for network... Attempt ");
    SerialMon.println(retryCount + 1);
    if (modem.waitForNetwork()) {
      SerialMon.println("Connected to Network!");
      break;
    }
    retryCount++;
    delay(5000);
  }

  if (retryCount >= maxRetries) {
    SerialMon.println("Failed to connect to network after multiple attempts.");
    return;
  }

  SerialMon.print("Connecting to APN: ");
  SerialMon.println(apn);

  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    SerialMon.println("GPRS connection failed!");
    return;
  }

  SerialMon.println("GPRS connected successfully.");
}
