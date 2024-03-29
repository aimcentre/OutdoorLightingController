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

enum eWifiStatus{ SUCCESS = 1, CONNECTION_INPROGRESS, CONNECTION_ACTIVE, CONNECTION_FAILED};
volatile eWifiStatus wifiStatus = SUCCESS;
volatile unsigned long wifiAttemtTimeStamp = 0;
volatile bool freshlyRebooted = true;  

volatile unsigned long gScheduleLoadFailCount = 0;
volatile bool gWasNightInPreviousCycle = false;

//The following flag is used to track whether any schedule-loading errors were handled since the last reboot.
//We set it to false in the initialization "setup()" function and use it in the lightingControlProcess() to 
//ensure we manually schedule the segment A to "ON" for a period of time if the schedule loading failed immediately
volatile bool gHandledScheduleLoadingErrorsSinceLastReboot = false;

#endif
 
