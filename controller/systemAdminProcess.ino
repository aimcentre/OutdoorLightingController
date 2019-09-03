
char *WifiSSID = NULL, *WifiPassword = NULL;

void systemAdminProcess(void * parameter) {
  
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
    
   
   
    delay(500);

  }
}
