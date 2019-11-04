// This file defines the data structures and default values used by the lighting controller

// **** IMPORTANT ****
//    If you change anything in this file, please make sure to change the value of the following CONFIG_VERSION constant to
//    something other than what is already stored for it in the ESP32 board. Otherwise, the changes will not be saved on the board.
//    To find what CONFIG_VERSION is in the board, please reset the board while monitoring its serial debugger output.
#define CONFIG_VERSION 2

// Maximum lengths of string values used for the fields of the configSettings_t structure. These maximum lenghts
// include the space for the null termination characters, meaning that the actual readable length of the string will
// be one less than the values defined below. Extra characters will be truncated. 
#define SSID_MAX_LENGTH 32
#define SSID_PASSWORD_MAX_LENGTH 16

#define HOST_NAME_MAX_LENGTH 64
#define URL_MAX_LENGTH 256

// Configuration settings structure definition
#ifndef CONFIG_SETTINGS_T
#define CONFIG_SETTINGS_T 

struct configSettings_t
{
  int configVersion;
  
  char accessPointSsid[SSID_MAX_LENGTH];
  char accessPointPassword[SSID_PASSWORD_MAX_LENGTH];

  char wifiSsid[SSID_MAX_LENGTH];
  char wifiPassword[SSID_PASSWORD_MAX_LENGTH];

  char scheduleApiHost[HOST_NAME_MAX_LENGTH];
  char scheduleApiPath[URL_MAX_LENGTH];

  unsigned int regularLampOnTime;
  unsigned int auxiliaryLampOnTime;

  unsigned int interSegmentDelay;

  unsigned int darknessThresholdHigh;
  unsigned int darknessThresholdLow;
  

  unsigned int scheduleCheckInterval;
};

configSettings_t settings;

#endif


// Enable configuring the portal over WIFI LAN. 
// WARNING: NOT SECURE. Set this to false in production environment
#define ENABLE_CONFIGRATION_OVER_LAN false
  
// Set the following directive to true to display passwords on the debugger
#define DEBUG_DISPLAY_CUSTOM_PASSWORDS false

// Default lighting parameters. All time periods are in seconds
#define REG_LAMP_ON_TIME 480
#define AUX_LAMP_ON_TIME 90
#define INTER_SEG_DELAY 10
#define DARKNESS_THRESHOLD 800

// Test-mode lighting parameters. All time periods are in seconds
#define TEST_REG_LAMP_ON_TIME 10
#define TEST_AUX_LAMP_ON_TIME 6
#define TEST_INTER_SEG_DELAY 4
#define TEST_DARKNESS_THRESHOLD 4095

#define SCHEDULE_CHECK_INTERVAL_SEC 900
