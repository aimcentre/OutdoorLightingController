// This file contains the status-LED control routines. 
// The status LED is an RGB LED mounted on the curcuit board.

#define R_LED 1
#define G_LED 2
#define B_LED 4

void systemMonitorProcess(void * parameter) {
  for(;;) {

    if(WiFi.status() != WL_CONNECTED || wifiInitialized == false)
    {
      Serial.println("Connecting to WiFi..");
      flash(R_LED, 50, 250, 8);
    }
    else
    {
      int dayLightLevel = analogRead(DAYLIGHT_SENSOR);
      Serial.printf("daylight status: %d\r\n", dayLightLevel);
  
      if(dayLightLevel < settings.dayLightThreshold)
        solid(B_LED, 500);
      else
        solid(G_LED, 500);
    }

    if(accessPointPasswordResetComplete)
    {
      solid(R_LED | B_LED, 4000);
      accessPointPasswordResetComplete = false;
    }
  }
}

void flash(int ledFlags, int onTime, int period, int iterations)
{
  for(int i = 0; i < iterations; ++i)
  {
    digitalWrite(STATUS_R, (ledFlags & R_LED) > 0);
    digitalWrite(STATUS_G, (ledFlags & G_LED) > 0);
    digitalWrite(STATUS_B, (ledFlags & B_LED) > 0);
    delay(onTime);

    digitalWrite(STATUS_R, false);
    digitalWrite(STATUS_G, false);
    digitalWrite(STATUS_B, false);
    delay(period - onTime);
  }
}

void solid(int ledFlags, int duration)
{
  digitalWrite(STATUS_R, (ledFlags & R_LED) > 0);
  digitalWrite(STATUS_G, (ledFlags & G_LED) > 0);
  digitalWrite(STATUS_B, (ledFlags & B_LED) > 0);
  delay(duration);
}
