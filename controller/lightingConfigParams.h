// Sensor GPIO pin assignments
// References: https://randomnerdtutorials.com/esp32-pinout-reference-gpios/
//             https://wiki.keyestudio.com/KS0413_keyestudio_ESP32_Core_Board

#define AIN_1 35
#define AIN_2 34
#define AIN_3 39
#define AIN_4 36

#define DIN_1 23
#define DIN_2 22
#define DIN_3 5
#define DIN_4 19
#define DIN_5 18
#define DIN_6 17
#define DIN_7 16
#define DIN_8 15

#define MOTION_A DIN_1
#define MOTION_B DIN_2
#define MOTION_C DIN_3
#define MOTION_D DIN_4
#define MOTION_E DIN_5
#define MOTION_F DIN_6

#define WIFI_RESET 13
#define DAYLIGHT_SENSOR AIN_1
#define TEMP_SENSOR_P AIN_3
#define TEMP_SENSOR_N AIN_4

#define STATUS_R 14
#define STATUS_G 2
#define STATUS_B 4

// Lamp-segment GPIO pin assignments
#define LAMP_A 27
#define LAMP_B 26
#define LAMP_C 25
#define LAMP_D 33
#define LAMP_E 32

// Lamp on/off output level 
#define ON HIGH
#define OFF LOW

#define DAYLIGHT_SENSOR AIN_1

// Data structure to keep track of when each motion sensor was triggered 
struct sensorState_t
{
  unsigned long clockA;
  unsigned long clockB;
  unsigned long clockC;
  unsigned long clockD;
  unsigned long clockE;
  unsigned long clockF;
};

struct lampState_t
{
  unsigned int offset;
  unsigned int period;
};

// Lighting control data structures and interrupt timer variables
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
volatile SemaphoreHandle_t timerSemaphore;

volatile sensorState_t sensorTriggerTimestamps{0, 0, 0, 0, 0, 0};
volatile lampState_t lampState1{0,0};
volatile lampState_t lampState2{0,0};
volatile lampState_t lampState3{0,0};
volatile lampState_t lampState4{0,0};
volatile lampState_t lampState5{0,0};

volatile unsigned long accessPointPasswordResetBtnPressedTime = 0;
volatile bool accessPointPasswordResetComplete = false;

volatile bool testMode = false;

#define SENSOR_STATES_HOSTORY_MAX 80
volatile sensorState_t sensorStateHistory[SENSOR_STATES_HOSTORY_MAX];
volatile int sensorStateHistoryLength = 0;
volatile int dayLightLevel = 0;

const char* host = "script.google.com";
String url = "/macros/s/AKfycbzM92oEPPacHXdE_Aq_YEhMkFd3q18OkqaEyAQXNZSxR0JYIJUz/exec";


 
