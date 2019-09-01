
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

    unsigned long t = millis();
    while(millis() - t < 10000 && digitalRead(WIFI_RESET) == false)
      delay(100);
    
    if(digitalRead(WIFI_RESET) == false)
    {
      Serial.println("Restting access-point password to default value ...");
      
      strncpy(settings.accessPointPassword, DEFAULT_AP_PW, SSID_PASSWORD_MAX_LENGTH-1);
      settings.accessPointPassword[SSID_PASSWORD_MAX_LENGTH-1] = '\0';

      //Saving the updated configuration structure in the EEPROM.
      EEPROM.put(0, settings);
      EEPROM.commit();
      delay(1000);
     
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
   
    delay(500);

  }
}
