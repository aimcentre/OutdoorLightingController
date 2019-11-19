#include "appConfig.h"

// Loads configuration settings from the EEPROM and assigns them to the given "settings" data structure.
// Resets configuration settings to default settings if the CONFIG_VERSION stated in the source code does not 
// match with the CONFIG_VERSION stored in the EEPROM. 

void loadConfigSettings(configSettings_t& settings)
{
  Serial.println("Loading configuration settings ...");
  EEPROM.get(0, settings);

  // Printing current settings into the serial debugger
  showSettings(settings);

  Serial.println("Loaded Config Version: " + String(settings.configVersion));
  Serial.println("Programmed Config Version: " + String(CONFIG_VERSION));

  Serial.println("Loaded AP SSID: " + String(settings.accessPointSsid));

  // Checking if the CONFIG_VERSION loaded from the EEPROM is different from the CONFIG_VERSION specified in the source code.
  if(settings.configVersion != CONFIG_VERSION)
  {
    // Reset configuration structure with default values specified in the source code.
    Serial.println();
    Serial.println("**** Resetting configuration settings ... ****");

    settings.configVersion = CONFIG_VERSION;
    
    strncpy(settings.accessPointSsid, AP_NAME, SSID_MAX_LENGTH-1); 
    settings.accessPointSsid[SSID_MAX_LENGTH-1] = '\0';
    
    strncpy(settings.accessPointPassword, DEFAULT_AP_PW, SSID_PASSWORD_MAX_LENGTH-1);
    settings.accessPointPassword[SSID_PASSWORD_MAX_LENGTH-1] = '\0';

    settings.wifiSsid[0] = 0; //Setting the WiFi SSID to null
    settings.wifiPassword[0] = 0; //Setting the WiFi Password to null

    settings.regularLampOnTime = REG_LAMP_ON_TIME;
    settings.auxiliaryLampOnTime = AUX_LAMP_ON_TIME;
    settings.interSegmentDelay = INTER_SEG_DELAY;
    settings.darknessThresholdHigh = DARKNESS_THRESHOLD;
    settings.darknessThresholdLow = DARKNESS_THRESHOLD - 100;
    settings.scheduleCheckInterval = SCHEDULE_CHECK_INTERVAL_SEC;
    settings.reportingInterval = REPORTING_INTERVAL_SEC;
    
    settings.segmentMask[0] = true;
    settings.segmentMask[1] = true;
    settings.segmentMask[2] = true;
    settings.segmentMask[3] = true;

    for(int i=0; i<5; ++i)
      settings.segmentMask[i] = true;

    strncpy(settings.scheduleApiHost, SCHEDULE_RETREIAVER_HOST, HOST_NAME_MAX_LENGTH-1); 
    settings.scheduleApiHost[HOST_NAME_MAX_LENGTH-1] = '\0';

    strncpy(settings.scheduleApiPath, SCHEDULE_RETRIEVER_URL, URL_MAX_LENGTH-1); 
    settings.scheduleApiPath[URL_MAX_LENGTH-1] = '\0';

    //Saving the updated configuration structure in the EEPROM.
    EEPROM.put(0, settings);
    EEPROM.commit();

    Serial.println("New configuration settings:");
    showSettings(settings);
    
  } // END: if(strcmp(settings.configVersion, CONFIG_VERSION) != 0)
}

// Displays settings in the serial debugger
void showSettings(configSettings_t& settings)
{
  Serial.println();
  Serial.println("Config version: " + String(settings.configVersion));
  
  Serial.println("Access Point SSID: " + String(settings.accessPointSsid));
  if(DEBUG_DISPLAY_CUSTOM_PASSWORDS || strcmp(settings.accessPointPassword, DEFAULT_AP_PW) == 0)
  {
    Serial.println("Access Point Password: " + String(settings.accessPointPassword));
  }
  else
  {
    if(strlen(settings.accessPointPassword) > 0)
      Serial.println("Access Point Password: <hidden>");
    else
      Serial.println("Access Point Password: no password set");
  } 
  
  
  Serial.println("WiFi SSID: " + String(settings.wifiSsid));
  if(DEBUG_DISPLAY_CUSTOM_PASSWORDS)
  {
    Serial.println("WiFi Password: " + String(settings.wifiPassword));
  }
  else
  {
    if(strlen(settings.wifiPassword) > 0)
      Serial.println("WiFi Password: <hidden>");
    else
      Serial.println("WiFi Password: no password set");
  }
  
  Serial.println();
}
