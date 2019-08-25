// Sensor GPIO pin assignments
#define MOTION_A 17
#define MOTION_B 16
#define MOTION_C 14
#define MOTION_D 12
#define MOTION_E 13
#define DAYLIGHT_SENSOR 15
#define WIFI_RESET 18

// Lamp-segment GPIO pin assignments
#define LAMP_A 22
#define LAMP_B 23
#define LAMP_C 32

// Lamp on/off output level 
#define ON HIGH
#define OFF LOW

// Day-time output level of the daylight sensor
#define DAYTIME HIGH 

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
