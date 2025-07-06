void timeFetch() {
  if (gps.date.isValid() && gps.time.isValid()) {
    // Populate the struct with GPS date and time
    myData.year = gps.date.year();
    myData.month = gps.date.month();
    myData.day = gps.date.day();
    myData.hour = gps.time.hour();
    myData.minute = gps.time.minute();
    myData.second = gps.time.second();
  }
}
