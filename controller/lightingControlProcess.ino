// References: lamp segments and sensor locatoins: https://www.lucidchart.com/documents/edit/67ebf293-cd7d-4d02-a396-586ca6e3912b/alWyEGoyqXJH?referringApp=google+drive&beaconFlowId=f2d16d7b76909438
// Triggering sequences: https://docs.google.com/spreadsheets/d/1fJMQOhua9JjkMa0Pfh4PbR-ifbYxZwPVFoGUY_oMN4c/edit#gid=0

#include "appConfig.h"

void IRAM_ATTR onTimer()
{  
  // Enable timer semaphore so that the main lighting-control loop is unlocked
  xSemaphoreGiveFromISR(timerSemaphore, NULL);
}

volatile unsigned long TickA = 0;
void IRAM_ATTR onMotionA()
{
  if(abs(millis() - TickA) < ACTION_JITTER_PERIOD_MILLISEC)
    return;
  TickA = millis();
  
  portENTER_CRITICAL(&resourceLock);
  gReport.AddAction(Report::eAction::MSA_TRIGGER);
  
  // Turnning on the lamp segment A immediately for the default duration
  gSegmentA.Trigger(0, settings.regularLampOnTime);

  // Scheduling lamp segment B to turn on for the auxiliary interval after the inter-segment delay
  gSegmentB.Trigger(settings.interSegmentDelay, settings.auxiliaryLampOnTime);

  portEXIT_CRITICAL(&resourceLock);

}

volatile unsigned long TickB = 0;
void IRAM_ATTR onMotionB()
{
  if(abs(millis() - TickB) < ACTION_JITTER_PERIOD_MILLISEC)
    return;
  TickB = millis();
 
   portENTER_CRITICAL(&resourceLock);
  gReport.AddAction(Report::eAction::MSB_TRIGGER);

  // Turnning on the lamp segments A and B immediately for the default duration
  gSegmentA.Trigger(0, settings.regularLampOnTime);
  gSegmentB.Trigger(0, settings.regularLampOnTime);

  // Scheduling lamp segment C to turn on for the auxiliary interval after the inter-segment delay
  gSegmentC.Trigger(settings.interSegmentDelay, settings.auxiliaryLampOnTime);
  
  portEXIT_CRITICAL(&resourceLock);
}

volatile unsigned long TickC = 0;
void IRAM_ATTR onMotionC()
{
  if(abs(millis() - TickC) < ACTION_JITTER_PERIOD_MILLISEC)
    return;
  TickC = millis();
 
  portENTER_CRITICAL(&resourceLock);
  gReport.AddAction(Report::eAction::MSC_TRIGGER);
  
  // Turnning on the lamp segment B and C immediately for the default duration
  gSegmentB.Trigger(0, settings.regularLampOnTime);
  gSegmentC.Trigger(0, settings.regularLampOnTime);
  
  portEXIT_CRITICAL(&resourceLock);
}

volatile unsigned long TickD = 0;
void IRAM_ATTR onMotionD()
{
  if(abs(millis() - TickD) < ACTION_JITTER_PERIOD_MILLISEC)
    return;
  TickD = millis();
 
  portENTER_CRITICAL(&resourceLock);
  gReport.AddAction(Report::eAction::MSD_TRIGGER);

  // Turnning on the lamp segment C immediately for the default duration
  gSegmentC.Trigger(0, settings.regularLampOnTime);

  // Turnning on the lamp segment B and D after halfo the inter-segent delay for the auxiliary interval
  gSegmentB.Trigger(settings.interSegmentDelay/2, settings.auxiliaryLampOnTime);
  gSegmentD.Trigger(settings.interSegmentDelay/2, settings.auxiliaryLampOnTime);
  
  portEXIT_CRITICAL(&resourceLock);
}

volatile unsigned long TickE = 0;
void IRAM_ATTR onMotionE()
{
  if(abs(millis() - TickE) < ACTION_JITTER_PERIOD_MILLISEC)
    return;
  TickE = millis();
 
  portENTER_CRITICAL(&resourceLock);
  gReport.AddAction(Report::eAction::MSE_TRIGGER);

  // Turnning on the lamp segment D immediately for the default duration
  gSegmentD.Trigger(0, settings.regularLampOnTime);

  // Turnning on the lamp segment C immediately for the auxiliary interval
  gSegmentC.Trigger(0, settings.auxiliaryLampOnTime);
  
  portEXIT_CRITICAL(&resourceLock);
}

volatile unsigned long TickF = 0;
void IRAM_ATTR onMotionF()
{
  if(abs(millis() - TickF) < ACTION_JITTER_PERIOD_MILLISEC)
    return;
  TickF = millis();
 
  portENTER_CRITICAL(&resourceLock);
  gReport.AddAction(Report::eAction::MSF_TRIGGER);

  // Turnning on the lamp segment D immediately for the default duration
  gSegmentD.Trigger(0, settings.regularLampOnTime);
  
  portEXIT_CRITICAL(&resourceLock);
}

volatile unsigned long TickG = 0;
void IRAM_ATTR onMotionG()
{
  if(abs(millis() - TickG) < ACTION_JITTER_PERIOD_MILLISEC)
    return;
  TickG = millis();
 
  portENTER_CRITICAL(&resourceLock);
  gReport.AddAction(Report::eAction::MSG_TRIGGER);

  // Turnning on the lamp segments B immediately for the default duration
  gSegmentB.Trigger(0, settings.regularLampOnTime);
  
  portEXIT_CRITICAL(&resourceLock);
}


void lightingControlProcess(void * parameter)
{ 
  unsigned long pingCounter = 0;
   
  for(;;)
  {
    if (xSemaphoreTake(timerSemaphore, 0) == pdTRUE)
    {
      gSunlightSensor.OnTick();
      bool isNight = gSunlightSensor.IsNight();

      if(gWasNightInPreviousCycle == false && isNight && (gScheduleLoadFailCount > 5)){
        //Here, the environment lighting turned from day to night AND the last attempt to load the 
        //schedule was failed in the last N number of times, consecuitively. In this case, we schedule
        //the lamp-segment-A for 5.5 hours from now.

        int turnOnTimeSec = 19800;
        Serial.printf("Getting dark but schedule loading had failed %d times, so proactively scheduling Segment A for %.2f hours from now.\n", gScheduleLoadFailCount, turnOnTimeSec/3600.0);
        gSegmentA.ScheduleCycle(0, turnOnTimeSec);
      }
      gWasNightInPreviousCycle = true;

      //Serial.printf("Is night: %d\r\n", isNight);
      int ambientDarkness = gSunlightSensor.getDarknessLevel();
      
      // Locking the timer mutex, decrementing and coying lamp-state timer values, and releasing the mutex
      portENTER_CRITICAL(&resourceLock);
      gSegmentA.OnTick(isNight);
      gSegmentB.OnTick(isNight);
      gSegmentC.OnTick(isNight);
      gSegmentD.OnTick(isNight);
      gSegmentE.OnTick(isNight);

      portEXIT_CRITICAL(&resourceLock);

      //Turning on and off lamps as necessary
      Report::eAction actions[5];
      if(gSegmentA.Execute(settings.segmentMask[0]))
        actions[0] = gSegmentA.GetStatus() == ON ? Report::eAction::L1_ON : Report::eAction::L1_OFF;
      else
        actions[0] = Report::eAction::NONE;

      if(gSegmentB.Execute(settings.segmentMask[1]))
        actions[1] = gSegmentB.GetStatus() == ON ? Report::eAction::L2_ON : Report::eAction::L2_OFF;
      else
        actions[1] = Report::eAction::NONE;

      if(gSegmentC.Execute(settings.segmentMask[2]))
        actions[2] = gSegmentC.GetStatus() == ON ? Report::eAction::L3_ON : Report::eAction::L3_OFF;
      else
        actions[2] = Report::eAction::NONE;

      if(gSegmentD.Execute(settings.segmentMask[3]))
        actions[3] = gSegmentD.GetStatus() == ON ? Report::eAction::L4_ON : Report::eAction::L4_OFF;
      else
        actions[3] = Report::eAction::NONE;

      if(gSegmentE.Execute(settings.segmentMask[4]))
        actions[4] = gSegmentE.GetStatus() == ON ? Report::eAction::L5_ON : Report::eAction::L5_OFF;
      else
        actions[4] = Report::eAction::NONE;

      portENTER_CRITICAL(&resourceLock);
      darknessLevel = ambientDarkness;
      for(int i=0; i<5; ++i)
      {
        if(actions[i] != Report::eAction::NONE)
          gReport.AddAction(actions[i]);
      }

      
      if(gReport.HasActivities() == false)
      {
        //If no action  has been found, so increment the ping counter.
        ++pingCounter;

        if(pingCounter > 3600 / PINGS_PER_HOUR)
        {
          //Ping counter exceeds the maximum count that the this loop can pass without reporting to the Motion Tracker.
          //Therefore, add a PING action to the report and reset the ping counter to 0
          gReport.AddAction(Report::eAction::PING);
          pingCounter = 0;
        }
      }
      else
      {
        //Already an acion has ben performed. Therefore, we do not specifically add a "PING" action because the data is sent to
        // the Motion Tracker by the already performed action, thus simply resent the ping counter.
        pingCounter = 0;
      }
      portEXIT_CRITICAL(&resourceLock);
             
    }
    else
      delay(10);
  }
}

void initLightingControlSystem()
{
  // Initialzing pin modes
  pinMode(DIN_1, INPUT_PULLDOWN);
  pinMode(DIN_2, INPUT_PULLDOWN);
  pinMode(DIN_3, INPUT_PULLDOWN);
  pinMode(DIN_4, INPUT_PULLDOWN);
  pinMode(DIN_5, INPUT_PULLDOWN);
  pinMode(DIN_6, INPUT_PULLDOWN);
  pinMode(DIN_7, INPUT_PULLDOWN);
  pinMode(DIN_8, INPUT_PULLDOWN);
  
  pinMode(AIN_1, INPUT);
  pinMode(AIN_2, INPUT);
  pinMode(AIN_3, INPUT);
  pinMode(AIN_4, INPUT);

  pinMode(STATUS_R, OUTPUT);
  pinMode(STATUS_G, OUTPUT);
  pinMode(STATUS_B, OUTPUT);

  pinMode(WIFI_RESET, INPUT);

  pinMode(LAMP_PIN_A, OUTPUT);
  pinMode(LAMP_PIN_B, OUTPUT);
  pinMode(LAMP_PIN_C, OUTPUT);
  pinMode(LAMP_PIN_D, OUTPUT);
  pinMode(LAMP_PIN_E, OUTPUT);

  digitalWrite(LAMP_PIN_A,  OFF);
  digitalWrite(LAMP_PIN_B,  OFF);
  digitalWrite(LAMP_PIN_C,  OFF);
  digitalWrite(LAMP_PIN_D,  OFF);
  digitalWrite(LAMP_PIN_E,  OFF);

  digitalWrite(STATUS_R,  OFF);
  digitalWrite(STATUS_G,  OFF);
  digitalWrite(STATUS_B,  OFF);
  
  
  // Create semaphore to inform us when the timer has fired
  timerSemaphore = xSemaphoreCreateBinary(); 

  // Use 1st timer of 4 (counted from zero).
  // Set 80 divider for prescaler, creating a 1MHz timer tick (see ESP32 Technical Reference Manual for more info).
  timer = timerBegin(0, 80, true);

  // Attach onTimer function to our timer.
  timerAttachInterrupt(timer, &onTimer, true);

  // Set alarm to call onTimer function every second (value in microseconds).
  // Repeat the alarm (third parameter)
  timerAlarmWrite(timer, 1000000, true);

  // Start an alarm
  timerAlarmEnable(timer);

  // Setting up motion-sensor interrupts
  attachInterrupt(MOTION_A, &onMotionA, FALLING);
  attachInterrupt(MOTION_B, &onMotionB, FALLING);
  attachInterrupt(MOTION_C, &onMotionC, FALLING);
  attachInterrupt(MOTION_D, &onMotionD, FALLING);
  attachInterrupt(MOTION_E, &onMotionE, FALLING);
  attachInterrupt(MOTION_F, &onMotionF, FALLING);
  attachInterrupt(MOTION_G, &onMotionG, FALLING);

  gSunlightSensor.Initialize(DAYLIGHT_SENSOR, settings.darknessThresholdHigh, settings.darknessThresholdLow);

/*
  // Setting up Access Point password-reset inturrupt
  attachInterrupt(WIFI_RESET, &onAccessPointPasswordResetBtnPressed, FALLING);
*/

  // If it's night time, turn all lamps for the default on time. Otherwise, turn them for 5 seconds.
  gSunlightSensor.OnTick();
  unsigned long t = gSunlightSensor.IsNight() ? settings.regularLampOnTime : 5;  
  Serial.printf("Initial on time: %d\r\n", t);
  gSegmentA.Trigger(2, t);
  gSegmentB.Trigger(2, t);
  gSegmentC.Trigger(2, t);
  gSegmentD.Trigger(2, t);
  gSegmentE.Trigger(2, t);
  
  
}
