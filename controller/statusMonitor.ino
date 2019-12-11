// This file contains the status-LED control routines. 
// The status LED is an RGB LED mounted on the curcuit board.

#define R_LED 1
#define G_LED 2
#define B_LED 4

#include <time.h>

void systemMonitorProcess(void * parameter) {
  for(;;) {

    //Serial.printf("Wifi Mode: %d    WiFi Status: %d\r\n", WiFi.getMode(), wifiStatus);
    //setTime(1575831028);
    //setTime(11, 52, 12, 8, 12, 2019);
    //int y = year;
    //Serial.printf("%4d-%2d-%2D  %2d:%2d:%2d\r\n", year(), month(), day(), hour(), minute(), second());


    
    //portENTER_CRITICAL(&resourceLock);
    //String activities = gReport.Export();
    //portEXIT_CRITICAL(&resourceLock);
    //Serial.println(activities);
    

    if(wifiStatus == eWifiStatus::CONNECTION_ACTIVE)
    {
      flash(R_LED | G_LED | B_LED, 50, 250, 8);
    }
    else if(wifiStatus == CONNECTION_INPROGRESS)
    {
      flash(R_LED, 50, 250, 8);
    }
    else
    {
      int darknessLevel = analogRead(DAYLIGHT_SENSOR);
      //Serial.printf("Ambient darkness: %d\r\n", darknessLevel);
  
      if(gSunlightSensor.IsNight())
      {
        solid(B_LED, 2000);
        //Serial.printf("Ambient level: %d Threshold: %d Color: Blue\r\n", darknessLevel, settings.dayLightThreshold);
      }
      else
      {
        solid(G_LED, 2000);
        //Serial.printf("Ambient darkness: %d Threshold: %d Color: Green\r\n", darknessLevel, settings.dayLightThreshold);
      }

      if(wifiStatus == CONNECTION_FAILED)
      {
        solid(R_LED, 100);
      }
    }

    if(accessPointPasswordResetComplete)
    {
      solid(R_LED | G_LED | B_LED, 4000);
      accessPointPasswordResetComplete = false;
    }

    
    //float temperature = getTemperature();
    //Serial.printf("Temperature: %f  ", temperature);
    //Serial.printf("Vals: %d, %d\r\n", analogRead(TEMP_SENSOR_P), analogRead(TEMP_SENSOR_N));
    //Serial.print("  AIN_2: "); Serial.println(analogRead(AIN_2));
  }
}

float getTemperature()
{
  int y_p = analogRead(TEMP_SENSOR_P);
  int y_n = analogRead(TEMP_SENSOR_N);
  int diff = y_p - y_n;
  
  float temperature = ((float) diff) * 330 / 4096;
  return temperature;
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
