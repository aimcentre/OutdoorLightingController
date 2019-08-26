// Sensor GPIO pin assignments
// References: https://randomnerdtutorials.com/esp32-pinout-reference-gpios/
//             https://wiki.keyestudio.com/KS0413_keyestudio_ESP32_Core_Board

#define MOTION_A 13
#define MOTION_B 14
#define MOTION_C 15
#define MOTION_D 16
#define MOTION_E 17
#define DAYLIGHT_SENSOR 18
#define WIFI_RESET 19

// Lamp-segment GPIO pin assignments
#define LAMP_A 25
#define LAMP_B 26
#define LAMP_C 27

// Lamp on/off output level 
#define ON HIGH
#define OFF LOW

// Day-time output level of the daylight sensor
#define DAYTIME LOW 

struct sensorState_t
{
  unsigned long clockA;
  unsigned long clockB;
  unsigned long clockC;
  unsigned long clockD;
  unsigned long clockE;
};

struct lampState_t
{
  int timerA;
  int timerB;
  int timerC;

  bool stateA;
  bool stateB;
  bool stateC;
};

// Lighting control data structures and interrupt timer variables
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
volatile SemaphoreHandle_t timerSemaphore;
volatile int isrCounter = 0;

volatile sensorState_t sensorTriggerTimestamps{0, 0, 0, 0, 0};
volatile lampState_t lampStateTimers{0, 0, 0, false, false, false};