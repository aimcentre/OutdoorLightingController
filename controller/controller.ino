// Main entry file of the lighting controller

// References:
//   Http Server: https://techtutorialsx.com/2018/10/12/esp32-http-web-server-handling-body-data/

#include <WiFi.h>
#include <EEPROM.h>
#include <ESPAsyncWebServer.h>

#include "systemConfigParams.h"
#include "lightingConfigParams.h"


AsyncWebServer server(80);

bool wifiInitialized = false;

void setup() 
{
  Serial.begin(115200);

  // Preparing the EEPROM Area on the flash
  EEPROM.begin(1024);

  // Loading configration settings from EEPROM
  loadConfigSettings(settings);

  // Turning on WiFi on both Access Point and Station modes.
  freshlyRebooted = true;
  WiFi.mode(WIFI_MODE_APSTA);
  delay(2000);
  Serial.print("Setting up Access Point …");
  WiFi.softAP(settings.accessPointSsid, settings.accessPointPassword);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  
  // Initializing WiFi network credentials
  //WiFi.begin(settings.wifiSsid, settings.wifiPassword);

  // Initializing the Access Point Hotspot and the system-configuration server
  initConfigServer();

  // Initializing timer interrupt system used for lighting control
  initLightingControlSystem();
  

 // Setting up lighting control routines to be executed on the core 0
  xTaskCreatePinnedToCore(
                    lightingControlProcess,   /* Task function. */
                    "AdminProcess", /* name of task. */
                    10000,          /* Stack size of task */
                    NULL,           /* parameter of the task */
                    1,              /* priority of the task */
                    NULL,           /* Task handle to keep track of created task */
                    0);             /* core */          

  // Setting up reporting process to be executed on the core 1
  xTaskCreatePinnedToCore(
                    systemAdminProcess,   /* Task function. */
                    "AdminProcess", /* name of task. */
                    10000,          /* Stack size of task */
                    NULL,           /* parameter of the task */
                    1,              /* priority of the task */
                    NULL,           /* Task handle to keep track of created task */
                    1);             /* core */

  // Setting up health-check routine to be executed on the core 1
  xTaskCreatePinnedToCore(
                    systemMonitorProcess,   /* Task function. */
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
  
  /*
  digitalWrite(STATUS_R, true);
  digitalWrite(STATUS_G, false);
  digitalWrite(STATUS_B, false);
  delay(500);

  digitalWrite(STATUS_R, false);
  digitalWrite(STATUS_G, true);
  digitalWrite(STATUS_B, false);
  delay(500);

  digitalWrite(STATUS_R, false);
  digitalWrite(STATUS_G, false);
  digitalWrite(STATUS_B, true);
  delay(500);

  digitalWrite(STATUS_R, true);
  digitalWrite(STATUS_G, false);
  digitalWrite(STATUS_B, true);
  delay(500);

  digitalWrite(STATUS_R, false);
  digitalWrite(STATUS_G, true);
  digitalWrite(STATUS_B, true);
  delay(500);

  digitalWrite(STATUS_R, true);
  digitalWrite(STATUS_B, true);
  delay(500);
  */
  
}
