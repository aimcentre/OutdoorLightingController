#include "hardwareConfiguration.h"
#include "LampSegment.h"
#include "MotionSensor.h"

#ifndef LIGHTING_PARAMETERS
#define LIGHTING_PARAMETERS

volatile LampSegment gSegmentA(LAMP_PIN_A);
volatile LampSegment gSegmentB(LAMP_PIN_B);
volatile LampSegment gSegmentC(LAMP_PIN_C);
volatile LampSegment gSegmentD(LAMP_PIN_D);
volatile LampSegment gSegmentE(LAMP_PIN_E);

volatile MotionSensor gSensorA(MOTION_A);
volatile MotionSensor gSensorB(MOTION_B);
volatile MotionSensor gSensorC(MOTION_C);
volatile MotionSensor gSensorD(MOTION_D);
volatile MotionSensor gSensorE(MOTION_E);
volatile MotionSensor gSensorF(MOTION_F);
volatile MotionSensor gSensorG(MOTION_G);

// Data structure to keep track of when each motion sensor was triggered 
struct sensorState_t
{
  unsigned long clockA;
  unsigned long clockB;
  unsigned long clockC;
  unsigned long clockD;
  unsigned long clockE;
  unsigned long clockF;
  unsigned long clockG;  
};

// Lighting control data structures and interrupt timer variables
hw_timer_t* timer = NULL;
portMUX_TYPE resourceLock = portMUX_INITIALIZER_UNLOCKED;
volatile SemaphoreHandle_t timerSemaphore;

hw_timer_t* lampScheduleTimer = NULL;

volatile sensorState_t sensorTriggerTimestamps{0, 0, 0, 0, 0, 0};

volatile unsigned long accessPointPasswordResetBtnPressedTime = 0;
volatile bool accessPointPasswordResetComplete = false;

volatile bool testMode = false;

#define SENSOR_STATES_HOSTORY_MAX 80
volatile sensorState_t sensorStateHistory[SENSOR_STATES_HOSTORY_MAX];
volatile int sensorStateHistoryLength = 0;
volatile int darknessLevel = 0;

const char* host = "script.google.com";
String url = "/macros/s/AKfycbzM92oEPPacHXdE_Aq_YEhMkFd3q18OkqaEyAQXNZSxR0JYIJUz/exec";

#endif
 
