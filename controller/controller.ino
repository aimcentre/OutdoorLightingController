// Main entry file of the lighting controller

// References:
//   Http Server: https://techtutorialsx.com/2018/10/12/esp32-http-web-server-handling-body-data/

#include <WiFi.h>
#include <EEPROM.h>
#include <ESPAsyncWebServer.h>

#include "configParams.h"

configSettings_t settings;
AsyncWebServer server(80);

bool WifiInitialized = false;

void setup() 
{
  Serial.begin(115200);

  // Preparing the EEPROM Area on the flash
  EEPROM.begin(1024);

  // Loading configration settings from EEPROM
  loadConfigSettings(settings);

  // Initializing WiFi network credentials
  WiFi.begin(settings.wifiSsid, settings.wifiPassword);

  // Initializing the Access Point Hotspot and the system-configuration server
  initConfigServer();

 // Setting up lighting control routines to be executed on the core 0
  xTaskCreatePinnedToCore(
                    lightingControlProcess,   /* Task function. */
                    "AdminProcess", /* name of task. */
                    10000,          /* Stack size of task */
                    NULL,           /* parameter of the task */
                    1,              /* priority of the task */
                    NULL,           /* Task handle to keep track of created task */
                    0);             /* core */          

  // Setting up reporting-processes and other health-check routines to be executed on the core 1
  xTaskCreatePinnedToCore(
                    reportingProcess,   /* Task function. */
                    "AdminProcess", /* name of task. */
                    10000,          /* Stack size of task */
                    NULL,           /* parameter of the task */
                    1,              /* priority of the task */
                    NULL,           /* Task handle to keep track of created task */
                    1);             /* core */                  
}

void loop() {
  // We don't use this default loop. We have it here simply because it's necessary for the Arduino framework.
  delay(10000);
}
