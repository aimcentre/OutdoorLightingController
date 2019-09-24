
// Lamp on/off output level 
#define ON HIGH
#define OFF LOW

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
volatile int darknessLevel = 0;

const char* host = "script.google.com";
String url = "/macros/s/AKfycbzM92oEPPacHXdE_Aq_YEhMkFd3q18OkqaEyAQXNZSxR0JYIJUz/exec";


 
