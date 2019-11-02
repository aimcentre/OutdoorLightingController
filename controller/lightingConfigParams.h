#include "hardwareConfiguration.h"
#include "LampSegment.h"
#include "Report.h"
#include "SunlightSensor.h"

#ifndef LIGHTING_PARAMETERS
#define LIGHTING_PARAMETERS

SunlightSensor gSunlightSensor;
                
volatile LampSegment gSegmentA(LAMP_PIN_A);
volatile LampSegment gSegmentB(LAMP_PIN_B);
volatile LampSegment gSegmentC(LAMP_PIN_C);
volatile LampSegment gSegmentD(LAMP_PIN_D);
volatile LampSegment gSegmentE(LAMP_PIN_E);

volatile Report gReport;

// Lighting control data structures and interrupt timer variables
hw_timer_t* timer = NULL;
portMUX_TYPE resourceLock = portMUX_INITIALIZER_UNLOCKED;
volatile SemaphoreHandle_t timerSemaphore;

hw_timer_t* lampScheduleTimer = NULL;

volatile unsigned long accessPointPasswordResetBtnPressedTime = 0;
volatile bool accessPointPasswordResetComplete = false;

volatile bool testMode = false;

volatile int darknessLevel = 0;

const char* spreadsheetHost = "script.google.com";
String url = "/macros/s/AKfycbzM92oEPPacHXdE_Aq_YEhMkFd3q18OkqaEyAQXNZSxR0JYIJUz/exec";


#endif
 
