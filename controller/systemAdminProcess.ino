#include <WiFiClientSecure.h>

char *WifiSSID = NULL, *WifiPassword = NULL;

void systemAdminProcess(void * parameter) {

  sensorState_t prevSensorStates{0, 0, 0, 0, 0, 0};
  sensorState_t currentSensorStates;
  for(;;) 
  {
    /*
    if(WiFi.status() != WL_CONNECTED || wifiInitialized == false)
    {
      int t = millis();
      while (WiFi.status() != WL_CONNECTED && (millis() - t) < 10000) {
        delay(1000);
        Serial.println("Connecting to WiFi..");
      }

      if(WiFi.status() == WL_CONNECTED)
      {
        Serial.println(WiFi.localIP());
        wifiInitialized = true;
      }
    }
    */

    // Checking if Access Point password-reset button is kept pressed (in which case it retrnse false) false for10 second
    unsigned long t = millis();
    while(digitalRead(WIFI_RESET) == false && millis() - t < 10000)
      delay(100);

    if(millis() - t >= 10000  && digitalRead(WIFI_RESET) == false)
    {
      Serial.println("Restting access-point password to default value ...");
      
      strncpy(settings.accessPointPassword, DEFAULT_AP_PW, SSID_PASSWORD_MAX_LENGTH-1);
      settings.accessPointPassword[SSID_PASSWORD_MAX_LENGTH-1] = '\0';

      //Saving the updated configuration structure in the EEPROM.
      EEPROM.put(0, settings);
      EEPROM.commit();
      delay(1000);

      // Resetting access point
      Serial.print("Resetting up Access Point …");
      WiFi.softAP(settings.accessPointSsid, settings.accessPointPassword);
      IPAddress IP = WiFi.softAPIP();
      Serial.print("AP IP address: ");
      Serial.println(IP);
     
      accessPointPasswordResetComplete = true;  
      accessPointPasswordResetBtnPressedTime = 0;
    }


    if(WiFi.status() != WL_CONNECTED)
      wifiInitialized = false;
    else if(!wifiInitialized)
    {
      Serial.print("Access Point IP: ");
      Serial.println(WiFi.localIP());
      wifiInitialized = true;
    }

    //if(WiFi.status() && 
    // TODO: check if sensor states hostory exists and if so save these states to the
    // spereadsheet https://docs.google.com/spreadsheets/d/1TkAlMnVnXBWtPnGmd1scvsLT7g7rjA1LxRSghtyn-48/edit#gid=0 by using its
    // Web App API Call: https://script.google.com/macros/s/AKfycbzM92oEPPacHXdE_Aq_YEhMkFd3q18OkqaEyAQXNZSxR0JYIJUz/exec?
    // where, append the motion sensor states asquery parameters. Use the query variable names to match exactly (case sensitive) the column names.
    // Check the App Script project of the spreadsheet for details.
    
   if(WiFi.status() == WL_CONNECTED)
   {
      int ambientDarkness = 0;
      portENTER_CRITICAL(&timerMux);
      ambientDarkness = darknessLevel;
      currentSensorStates.clockA = sensorTriggerTimestamps.clockA;
      currentSensorStates.clockB = sensorTriggerTimestamps.clockB;
      currentSensorStates.clockC = sensorTriggerTimestamps.clockC;
      currentSensorStates.clockD = sensorTriggerTimestamps.clockD;
      currentSensorStates.clockE = sensorTriggerTimestamps.clockE;
      currentSensorStates.clockF = sensorTriggerTimestamps.clockF;
      portEXIT_CRITICAL(&timerMux);
      
      if(prevSensorStates.clockA != currentSensorStates.clockA ||
         prevSensorStates.clockB != currentSensorStates.clockB ||
         prevSensorStates.clockC != currentSensorStates.clockC ||
         prevSensorStates.clockD != currentSensorStates.clockD ||
         prevSensorStates.clockE != currentSensorStates.clockE ||
         prevSensorStates.clockF != currentSensorStates.clockF)
       {
          WiFiClientSecure client;
          const int httpPort = 443;
          if (!client.connect(host, httpPort)) {
            Serial.println("connection failed");
            return;
          }
          float temperature = getTemperature();
          String params = String("?m1=") + currentSensorStates.clockA + 
                          "&m2=" + currentSensorStates.clockB + 
                          "&m3=" + currentSensorStates.clockC + 
                          "&m4=" + currentSensorStates.clockD + 
                          "&m5=" + currentSensorStates.clockE + 
                          "&m6=" + currentSensorStates.clockF + 
                          "&d=" + ambientDarkness + 
                          "&t=" + temperature;
          String dataEncodedUrl = url + params;
          //Serial.println(dataEncodedUrl);
          client.print(String("GET ") + dataEncodedUrl +" HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");

          prevSensorStates.clockA = currentSensorStates.clockA;
          prevSensorStates.clockB = currentSensorStates.clockB;
          prevSensorStates.clockC = currentSensorStates.clockC;
          prevSensorStates.clockD = currentSensorStates.clockD;
          prevSensorStates.clockE = currentSensorStates.clockE;
          prevSensorStates.clockF = currentSensorStates.clockF;
       }
   }
   
    delay(500);

  }
}
