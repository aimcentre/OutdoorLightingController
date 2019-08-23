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

// Configuration settings structure definition
struct configSettings_t
{
  int configVersion;
  
  char accessPointSsid[SSID_MAX_LENGTH];
  char accessPointPassword[SSID_PASSWORD_MAX_LENGTH];

  char wifiSsid[SSID_MAX_LENGTH];
  char wifiPassword[SSID_PASSWORD_MAX_LENGTH];
};

// Default Access Point name and the password
#define AP_NAME "AimLightingControllerAP"
#define DEFAULT_AP_PW "password"
