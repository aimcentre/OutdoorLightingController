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
      Report::eAction actions[REPORT_BUFFER_SIZE];
      unsigned long timestamps[REPORT_BUFFER_SIZE];
      int numReportEntries = 0;
      
      portENTER_CRITICAL(&resourceLock);
      ambientDarkness = darknessLevel;
      numReportEntries = gReport.ExportTriggers(actions, timestamps);
      portEXIT_CRITICAL(&resourceLock);

      
/*      
      if(prevSensorStates.clockA != currentSensorStates.clockA ||
         prevSensorStates.clockB != currentSensorStates.clockB ||
         prevSensorStates.clockC != currentSensorStates.clockC ||
         prevSensorStates.clockD != currentSensorStates.clockD ||
         prevSensorStates.clockE != currentSensorStates.clockE ||
         prevSensorStates.clockF != currentSensorStates.clockF ||
         prevSensorStates.clockG != currentSensorStates.clockG)
*/        
      if(numReportEntries > 0)
      {
          WiFiClientSecure client;
          const int httpPort = 443;
          if (!client.connect(host, httpPort)) 
          {
            Serial.println("connection failed");
            return;
          }

          float temperature = getTemperature();
          
          for(int i=0; i<numReportEntries; ++i)
          {
            String params = String("?t=") + temperature + 
                            "&clk=" + timestamps[i] +
                            "&d=" + ambientDarkness;
            switch(actions[i])
            {
              case Report::eAction::MSA_TRIGGER: params = params + "&mA=1"; break;
              case Report::eAction::MSB_TRIGGER: params = params + "&mB=1"; break;
              case Report::eAction::MSC_TRIGGER: params = params + "&mC=1"; break;
              case Report::eAction::MSD_TRIGGER: params = params + "&mD=1"; break;
              case Report::eAction::MSE_TRIGGER: params = params + "&mE=1"; break;
              case Report::eAction::MSF_TRIGGER: params = params + "&mF=1"; break;
              case Report::eAction::MSG_TRIGGER: params = params + "&mG=1"; break;

              case Report::eAction::L1_ON: params = params  + "&s1=1"; break;
              case Report::eAction::L2_ON: params = params  + "&s2=1"; break;
              case Report::eAction::L3_ON: params = params  + "&s3=1"; break;
              case Report::eAction::L4_ON: params = params  + "&s4=1"; break;
              case Report::eAction::L1_OFF: params = params + "&s1=0"; break;
              case Report::eAction::L2_OFF: params = params + "&s2=0"; break;
              case Report::eAction::L3_OFF: params = params + "&s3=0"; break;
              case Report::eAction::L4_OFF: params = params + "&s4=0"; break;
                
            }

            String dataEncodedUrl = url + params;
            //Serial.println(dataEncodedUrl);
            client.print(String("GET ") + dataEncodedUrl +" HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
          }
          
/*          
          String params = String("?mA=") + currentSensorStates.clockA + 
                          "&mB=" + currentSensorStates.clockB + 
                          "&mC=" + currentSensorStates.clockC + 
                          "&mD=" + currentSensorStates.clockD + 
                          "&mE=" + currentSensorStates.clockE + 
                          "&mF=" + currentSensorStates.clockF + 
                          "&mG=" + currentSensorStates.clockG + 
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
          prevSensorStates.clockG = currentSensorStates.clockG;
 */         
       }
   }
   
    delay(500);

  }
}
