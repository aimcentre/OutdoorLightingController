// References: lamp segments and sensor locatoins: https://www.lucidchart.com/documents/edit/67ebf293-cd7d-4d02-a396-586ca6e3912b/alWyEGoyqXJH?referringApp=google+drive&beaconFlowId=f2d16d7b76909438
// Triggering sequences: https://docs.google.com/spreadsheets/d/1fJMQOhua9JjkMa0Pfh4PbR-ifbYxZwPVFoGUY_oMN4c/edit#gid=0

void IRAM_ATTR onTimer()
{  
  // Enable timer semaphore so that the main lighting-control loop is unlocked
  xSemaphoreGiveFromISR(timerSemaphore, NULL);
}

void IRAM_ATTR onMotionA()
{
  portENTER_CRITICAL(&resourceLock);
  gReport.AddAction(Report::eAction::MSA_TRIGGER);
  
  // Turnning on the lamp segment A immediately for the default duration
  gSegmentA.Trigger(0, settings.regularLampOnTime);

  // Scheduling lamp segment B to turn on for the auxiliary interval after the inter-segment delay
  gSegmentB.Trigger(settings.interSegmentDelay, settings.auxiliaryLampOnTime);

  portEXIT_CRITICAL(&resourceLock);

}

void IRAM_ATTR onMotionB()
{
  portENTER_CRITICAL(&resourceLock);
  gReport.AddAction(Report::eAction::MSB_TRIGGER);

  // Turnning on the lamp segments A and B immediately for the default duration
  gSegmentA.Trigger(0, settings.regularLampOnTime);
  gSegmentB.Trigger(0, settings.regularLampOnTime);
  
  portEXIT_CRITICAL(&resourceLock);
}

void IRAM_ATTR onMotionC()
{
  portENTER_CRITICAL(&resourceLock);
  gReport.AddAction(Report::eAction::MSC_TRIGGER);
  
  // Turnning on the lamp segment B immediately for the default duration
  gSegmentB.Trigger(0, settings.regularLampOnTime);

  // Scheduling lamp segment C to turn on for the auxiliary interval shortly after
  gSegmentC.Trigger(settings.interSegmentDelay/2, settings.auxiliaryLampOnTime);
  
  portEXIT_CRITICAL(&resourceLock);
}

void IRAM_ATTR onMotionD()
{
  portENTER_CRITICAL(&resourceLock);
  gReport.AddAction(Report::eAction::MSD_TRIGGER);

  // Turnning on the lamp segment C immediately for the default duration
  gSegmentC.Trigger(0, settings.regularLampOnTime);

  // Turnning on the lamp segment D immediately for the auxiliary interval
  gSegmentD.Trigger(0, settings.auxiliaryLampOnTime);
  
  portEXIT_CRITICAL(&resourceLock);
}

void IRAM_ATTR onMotionE()
{
  portENTER_CRITICAL(&resourceLock);
  gReport.AddAction(Report::eAction::MSE_TRIGGER);

  // Turnning on the lamp segment D immediately for the default duration
  gSegmentD.Trigger(0, settings.regularLampOnTime);

  // Turnning on the lamp segment C immediately for the auxiliary interval
  gSegmentC.Trigger(0, settings.auxiliaryLampOnTime);
  
  portEXIT_CRITICAL(&resourceLock);
}

void IRAM_ATTR onMotionF()
{
  portENTER_CRITICAL(&resourceLock);
  gReport.AddAction(Report::eAction::MSF_TRIGGER);

  // Turnning on the lamp segment D immediately for the default duration
  gSegmentD.Trigger(0, settings.regularLampOnTime);
  
  portEXIT_CRITICAL(&resourceLock);
}

void IRAM_ATTR onMotionG()
{
  portENTER_CRITICAL(&resourceLock);
  gReport.AddAction(Report::eAction::MSG_TRIGGER);

  // Turnning on the lamp segments B immediately for the default duration
  gSegmentB.Trigger(0, settings.regularLampOnTime);
  
  portEXIT_CRITICAL(&resourceLock);
}

void lightingControlProcess(void * parameter)
{
  sensorState_t sensorTriggers;
   
  for(;;)
  {
    if (xSemaphoreTake(timerSemaphore, 0) == pdTRUE)
    {
      int ambientDarkness = getDarknessLevel();
      
      // Locking the timer mutex, decrementing and coying lamp-state timer values, and releasing the mutex
      portENTER_CRITICAL(&resourceLock);
      gSegmentA.OnTick(ambientDarkness, settings.dayLightThreshold);
      gSegmentB.OnTick(ambientDarkness, settings.dayLightThreshold);
      gSegmentC.OnTick(ambientDarkness, settings.dayLightThreshold);
      gSegmentD.OnTick(ambientDarkness, settings.dayLightThreshold);
      gSegmentE.OnTick(ambientDarkness, settings.dayLightThreshold);

      darknessLevel = ambientDarkness;
      portEXIT_CRITICAL(&resourceLock);

      //Turning on and off lamps as necessary
      if(gSegmentA.Execute())
        gReport.AddAction(gSegmentA.GetStatus() == ON ? Report::eAction::L1_ON : Report::eAction::L1_OFF);
      
      if(gSegmentB.Execute())
        gReport.AddAction(gSegmentB.GetStatus() == ON ? Report::eAction::L2_ON : Report::eAction::L2_OFF);

      if(gSegmentC.Execute())
        gReport.AddAction(gSegmentC.GetStatus() == ON ? Report::eAction::L3_ON : Report::eAction::L3_OFF);

      if(gSegmentD.Execute())
        gReport.AddAction(gSegmentD.GetStatus() == ON ? Report::eAction::L4_ON : Report::eAction::L4_OFF);

       if(gSegmentE.Execute())
        gReport.AddAction(gSegmentE.GetStatus() == ON ? Report::eAction::L5_ON : Report::eAction::L5_OFF);      
    }
    else
      delay(10);
  }
  
}

int getDarknessLevel()
{
  return analogRead(DAYLIGHT_SENSOR);
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

  // Initializing the lamp-schedule timer
  lampScheduleTimer = timerBegin(1, 80, true);
  timerAttachInterrupt(lampScheduleTimer, &fetchSchedule, true);
  timerAlarmWrite(lampScheduleTimer, 2000000, true);
  timerAlarmEnable(lampScheduleTimer);


  // Setting up motion-sensor interrupts
  attachInterrupt(MOTION_A, &onMotionA, FALLING);
  attachInterrupt(MOTION_B, &onMotionB, FALLING);
  attachInterrupt(MOTION_C, &onMotionC, FALLING);
  attachInterrupt(MOTION_D, &onMotionD, FALLING);
  attachInterrupt(MOTION_E, &onMotionE, FALLING);
  attachInterrupt(MOTION_F, &onMotionF, FALLING);
  attachInterrupt(MOTION_G, &onMotionG, FALLING);

/*
  // Setting up Access Point password-reset inturrupt
  attachInterrupt(WIFI_RESET, &onAccessPointPasswordResetBtnPressed, FALLING);
*/

  // Turnning all lamps for 5 second
  gSegmentA.Trigger(0, 5);
  gSegmentB.Trigger(0, 5);
  gSegmentC.Trigger(0, 5);
  gSegmentD.Trigger(0, 5);
  gSegmentE.Trigger(0, 5);
  
  
}

void fetchSchedule()
{
  const char* timeAPI = "http://worldtimeapi.org/api/timezone/America/Edmonton";
  
  Serial.println("Fetching schedule ...");
  
  if(WiFi.status() == WL_CONNECTED)
  {
  }
}
