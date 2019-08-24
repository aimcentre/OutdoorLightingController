
char *WifiSSID = NULL, *WifiPassword = NULL;

void reportingProcess( void * parameter) {
  for(;;) {

    if(WiFi.status() != WL_CONNECTED || WifiInitialized == false)
    {
      while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi..");
      }
      Serial.println(WiFi.localIP());
      WifiInitialized = true;
    }
    
    delay(500);

  }
}
