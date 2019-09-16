void IRAM_ATTR turnLampSegmentOn(volatile lampState_t* lampState, unsigned int offset, unsigned int period);

// References: lamp segments and sensor locatoins: https://www.lucidchart.com/documents/edit/67ebf293-cd7d-4d02-a396-586ca6e3912b/alWyEGoyqXJH?referringApp=google+drive&beaconFlowId=f2d16d7b76909438
// Triggering sequences: https://docs.google.com/spreadsheets/d/1fJMQOhua9JjkMa0Pfh4PbR-ifbYxZwPVFoGUY_oMN4c/edit#gid=0

void IRAM_ATTR onTimer()
{  
  // Enable timer semaphore so that the main lighting-control loop is unlocked
  xSemaphoreGiveFromISR(timerSemaphore, NULL);
}

void IRAM_ATTR onMotionA()
{
  portENTER_CRITICAL(&timerMux);
  sensorTriggerTimestamps.clockA = millis();
  
  // Turnning on the lamp segment 1 immediately for the default duration
  turnLampSegmentOn(&lampState1, 0, settings.regularLampOnTime);

  // Scheduling lamp segment 2 to turn on for the auxiliary interval after the inter-segment delay
  turnLampSegmentOn(&lampState2, settings.interSegmentDelay, settings.auxiliaryLampOnTime);

  portEXIT_CRITICAL(&timerMux);
}

void IRAM_ATTR onMotionB()
{
  portENTER_CRITICAL(&timerMux);
  sensorTriggerTimestamps.clockB = millis();

  // Turnning on the lamp segments 1 and 2 immediately for the default duration
  turnLampSegmentOn(&lampState1, 0, settings.regularLampOnTime);
  turnLampSegmentOn(&lampState2, 0, settings.regularLampOnTime);
  
  portEXIT_CRITICAL(&timerMux);
}

void IRAM_ATTR onMotionC()
{
  portENTER_CRITICAL(&timerMux);
  sensorTriggerTimestamps.clockC = millis();
  
  // Turnning on the lamp segment 2 immediately for the default duration
  turnLampSegmentOn(&lampState2, 0, settings.regularLampOnTime);

  // Scheduling lamp segment 3 to turn on for the auxiliary interval shortly after
  turnLampSegmentOn(&lampState3, settings.interSegmentDelay/2, settings.auxiliaryLampOnTime);
  
  portEXIT_CRITICAL(&timerMux);
}

void IRAM_ATTR onMotionD()
{
  portENTER_CRITICAL(&timerMux);
  sensorTriggerTimestamps.clockD = millis();

  // Turnning on the lamp segment 3 immediately for the default duration
  turnLampSegmentOn(&lampState3, 0, settings.regularLampOnTime);

  // Turnning on the lamp segment 4 immediately for the auxiliary interval
  turnLampSegmentOn(&lampState4, 0, settings.auxiliaryLampOnTime);
  
  portEXIT_CRITICAL(&timerMux);
}

void IRAM_ATTR onMotionE()
{
  portENTER_CRITICAL(&timerMux);
  sensorTriggerTimestamps.clockE = millis();

  // Turnning on the lamp segment 4 immediately for the default duration
  turnLampSegmentOn(&lampState4, 0, settings.regularLampOnTime);

  // Turnning on the lamp segment 3 immediately for the auxiliary interval
  turnLampSegmentOn(&lampState3, 0, settings.auxiliaryLampOnTime);
  
  portEXIT_CRITICAL(&timerMux);
}

void IRAM_ATTR onMotionF()
{
  portENTER_CRITICAL(&timerMux);
  sensorTriggerTimestamps.clockF = millis();

  // Turnning on the lamp segment 4 immediately for the default duration
  turnLampSegmentOn(&lampState4, 0, settings.regularLampOnTime);
  
  portEXIT_CRITICAL(&timerMux);
}

void IRAM_ATTR onAccessPointPasswordResetBtnPressed()
{
  portENTER_CRITICAL(&timerMux);
  accessPointPasswordResetBtnPressedTime = millis();
  portEXIT_CRITICAL(&timerMux);
}

void lightingControlProcess(void * parameter)
{
  sensorState_t sensorTriggers;
  lampState_t segA, segB, segC, segD, segE;
  bool prevOnA = false, prevOnB = false, prevOnC = false, prevOnD = false, prevOnE = false;
   
  for(;;)
  {
    if (xSemaphoreTake(timerSemaphore, 0) == pdTRUE)
    {
      int lightLevel = analogRead(DAYLIGHT_SENSOR);
      
      // Locking the timer mutex, decrementing and coying lamp-state timer values, and releasing the mutex
      portENTER_CRITICAL(&timerMux);

      dayLightLevel = lightLevel;
      segA.offset = lampState1.offset > 0 ? lampState1.offset-- : 0;
      if(segA.offset == 0)
        segA.period = lampState1.period > 0 ? lampState1.period-- : 0;

      segB.offset = lampState2.offset > 0 ? lampState2.offset-- : 0;
      if(segB.offset == 0)
        segB.period = lampState2.period > 0 ? lampState2.period-- : 0;
      
      segC.offset = lampState3.offset > 0 ? lampState3.offset-- : 0;
      if(segC.offset == 0)
        segC.period = lampState3.period > 0 ? lampState3.period-- : 0;

      segD.offset = lampState4.offset > 0 ? lampState4.offset-- : 0;
      if(segD.offset == 0)
        segD.period = lampState4.period > 0 ? lampState4.period-- : 0;
        
      segE.offset = lampState5.offset > 0 ? lampState5.offset-- : 0;
      if(segE.offset == 0)
        segE.period = lampState5.period > 0 ? lampState5.period-- : 0;

      portEXIT_CRITICAL(&timerMux);

      // Checking the new states of lamp segments depending on whether the corresponding timer values are positive or zero
      bool isNight = lightLevel < settings.dayLightThreshold;
      bool turnOnA = isNight && segA.offset == 0 && segA.period > 0;
      bool turnOnB = isNight && segB.offset == 0 && segB.period > 0;
      bool turnOnC = isNight && segC.offset == 0 && segC.period > 0;
      bool turnOnD = isNight && segD.offset == 0 && segD.period > 0;
      bool turnOnE = isNight && segE.offset == 0 && segE.period > 0;

      if(prevOnA != turnOnA || prevOnB != turnOnB || prevOnC != turnOnC || prevOnD != turnOnD || prevOnE != turnOnE)
      {
        digitalWrite(LAMP_A, turnOnA ? ON : OFF);
        digitalWrite(LAMP_B, turnOnB ? ON : OFF);
        digitalWrite(LAMP_C, turnOnC ? ON : OFF);
        digitalWrite(LAMP_D, turnOnD ? ON : OFF);
        digitalWrite(LAMP_E, turnOnE ? ON : OFF);

        Serial.printf("Segment A: %s, Segment B: %s, Segment C: %s, Segment D: %s, Segment E: %s\r\n", turnOnA ? "ON" : "OFF", turnOnB ? "ON" : "OFF", turnOnC ? "ON" : "OFF", turnOnD ? "ON" : "OFF", turnOnE ? "ON" : "OFF");
      }

      prevOnA = turnOnA;
      prevOnB = turnOnB;
      prevOnC = turnOnC;
      prevOnD = turnOnD;
      prevOnE = turnOnE;
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

  pinMode(LAMP_A, OUTPUT);
  pinMode(LAMP_B, OUTPUT);
  pinMode(LAMP_C, OUTPUT);
  pinMode(LAMP_D, OUTPUT);
  pinMode(LAMP_E, OUTPUT);

  digitalWrite(LAMP_A,  OFF);
  digitalWrite(LAMP_B,  OFF);
  digitalWrite(LAMP_C,  OFF);
  digitalWrite(LAMP_D,  OFF);
  digitalWrite(LAMP_E,  OFF);

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
  attachInterrupt(MOTION_A, &onMotionA,   FALLING);
  attachInterrupt(MOTION_B, &onMotionB,   FALLING);
  attachInterrupt(MOTION_C, &onMotionC,   FALLING);
  attachInterrupt(MOTION_D, &onMotionD, FALLING);
  attachInterrupt(MOTION_E, &onMotionE, FALLING);
  attachInterrupt(MOTION_F, &onMotionF, FALLING);

  // Setting up Access Point password-reset inturrupt
  attachInterrupt(WIFI_RESET, &onAccessPointPasswordResetBtnPressed, FALLING);

  // Turnning all lamps for 5 second
  turnLampSegmentOn(&lampState1, 0, 5);
  turnLampSegmentOn(&lampState2, 0, 5);
  turnLampSegmentOn(&lampState3, 0, 5);
  turnLampSegmentOn(&lampState4, 0, 5);
  turnLampSegmentOn(&lampState5, 0, 5);
  
}

void IRAM_ATTR turnLampSegmentOn(volatile lampState_t* lampState, unsigned int offset, unsigned int period)
{
  if(lampState->period == 0)
  {
    // Segment is currently off, so set it to turn on for the given period after the given delay.
    lampState->offset = offset;
    lampState->period = period;
  }
  else
  {
    // The segment is either currently on or it is scheduled to be on
    
    if(lampState->offset == 0)
    {
      // Segment is currently turned on. Keep it turned on for current period or the given offset+period, whichever is longer
      unsigned int t = lampState->period;
      lampState->period = max (t, offset + period);    
    }
    else
    {
      // The segment is scheduled to be turned on after the delay specified by lampState->offset. 
      // Turn it on after that delay or the given offset, whichever is smaller,
      // and then keep it on past the scheduled period or the the given offset+period, whichever is larger
      int time_to_turn_off = max(lampState->offset + lampState->period, offset + period);
      unsigned int t = lampState->offset;
      lampState->offset = min(t, offset);
      lampState->period = time_to_turn_off - lampState->offset;
    } 
  }
}

/*
void IRAM_ATTR turnLampSegmentOn(volatile lampState_t* lampState, unsigned int period)
{
  if(lampState->period == 0)
  {
    // The lamp is already off. Turn it on immediately
    lampState->offset = 0;
    lampState->period = period;
  }
  else
  {
    // The lamp is already on or scheduled to be on. Turn it on immediately but keep it for the turn-on duration or 
    // the end of the scheduled turn-on period, whichever is longer
    unsigned int t = lampState->offset + lampState->period;
    int time_to_turn_off = max(t, period);
    lampState->offset = 0;
    lampState->period = time_to_turn_off;
  }
}
*/
