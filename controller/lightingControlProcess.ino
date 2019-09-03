void IRAM_ATTR scheduleLampSegmentOn(volatile lampState_t* lampState, unsigned int offset, unsigned int period);
void IRAM_ATTR turnLampSegmentOn(volatile lampState_t* lampState, unsigned int period);

void IRAM_ATTR onTimer()
{  
  // Enable timer semaphore so that the main lighting-control loop is unlocked
  xSemaphoreGiveFromISR(timerSemaphore, NULL);
}

void IRAM_ATTR onMotionA()
{
  portENTER_CRITICAL(&timerMux);
  sensorTriggerTimestamps.clockA = millis();
  
  // Turnning on the lamp segment A immediately for the default duration
  turnLampSegmentOn(&lampStateA, settings.regularLampOnTime);

  // Scheduling lamp segment B to turn on for the auxiliary interval after the inter-segment delay
  scheduleLampSegmentOn(&lampStateB, settings.interSegmentDelay, settings.auxiliaryLampOnTime);

  portEXIT_CRITICAL(&timerMux);
}

void IRAM_ATTR onMotionB()
{
  portENTER_CRITICAL(&timerMux);
  sensorTriggerTimestamps.clockB = millis();

  // Turnning on the lamp segments A and B immediately for the default duration
  turnLampSegmentOn(&lampStateA, settings.regularLampOnTime);
  turnLampSegmentOn(&lampStateB, settings.regularLampOnTime);
  
  portEXIT_CRITICAL(&timerMux);
}

void IRAM_ATTR onMotionC()
{
  portENTER_CRITICAL(&timerMux);
  sensorTriggerTimestamps.clockC = millis();
  
  // Turnning on the lamp segment B immediately for the default duration
  turnLampSegmentOn(&lampStateB, settings.regularLampOnTime);

  // Scheduling lamp segment A to turn on for the auxiliary interval after the inter-segment delay
  scheduleLampSegmentOn(&lampStateA, settings.interSegmentDelay, settings.auxiliaryLampOnTime);
  
  portEXIT_CRITICAL(&timerMux);
}

void IRAM_ATTR onMotionD()
{
  portENTER_CRITICAL(&timerMux);
  sensorTriggerTimestamps.clockD = millis();

  // Turnning on the lamp segment C immediately for the default duration
  turnLampSegmentOn(&lampStateC, settings.regularLampOnTime);

  // Turnning on the lamp segment D immediately for the auxiliary interval
  turnLampSegmentOn(&lampStateD, settings.auxiliaryLampOnTime);

  // Scheduling lamp segment B to turn on for the auxiliary interval after the inter-segment delay
  scheduleLampSegmentOn(&lampStateB, settings.interSegmentDelay, settings.auxiliaryLampOnTime);
  
  portEXIT_CRITICAL(&timerMux);
}

void IRAM_ATTR onMotionE()
{
  portENTER_CRITICAL(&timerMux);
  sensorTriggerTimestamps.clockE = millis();

  // Turnning on the lamp segment D immediately for the default duration
  turnLampSegmentOn(&lampStateD, settings.regularLampOnTime);

  // Turnning on the lamp segment C immediately for the auxiliary interval
  turnLampSegmentOn(&lampStateC, settings.auxiliaryLampOnTime);
  
  portEXIT_CRITICAL(&timerMux);
}

void IRAM_ATTR onMotionF()
{
  portENTER_CRITICAL(&timerMux);
  sensorTriggerTimestamps.clockF = millis();

  // Turnning on the lamp segment D immediately for the default duration
  turnLampSegmentOn(&lampStateD, settings.regularLampOnTime);
  
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
  lampState_t segA, segB, segC, segD;
  bool prevOnA = false, prevOnB = false, prevOnC = false, prevOnD = false;
   
  for(;;)
  {
    if (xSemaphoreTake(timerSemaphore, 0) == pdTRUE)
    {
      // Locking the timer mutex, decrementing and coying lamp-state timer values, and releasing the mutex
      portENTER_CRITICAL(&timerMux);

      segA.offset = lampStateA.offset > 0 ? lampStateA.offset-- : 0;
      if(segA.offset == 0)
        segA.period = lampStateA.period > 0 ? lampStateA.period-- : 0;

      segB.offset = lampStateB.offset > 0 ? lampStateB.offset-- : 0;
      if(segB.offset == 0)
        segB.period = lampStateB.period > 0 ? lampStateB.period-- : 0;
      
      segC.offset = lampStateC.offset > 0 ? lampStateC.offset-- : 0;
      if(segC.offset == 0)
        segC.period = lampStateC.period > 0 ? lampStateC.period-- : 0;

      segD.offset = lampStateD.offset > 0 ? lampStateD.offset-- : 0;
      if(segD.offset == 0)
        segD.period = lampStateD.period > 0 ? lampStateD.period-- : 0;
        

      portEXIT_CRITICAL(&timerMux);

      // Checking the new states of lamp segments depending on whether the corresponding timer values are positive or zero
      int dayLightLevel = analogRead(DAYLIGHT_SENSOR);
      bool isNight = dayLightLevel < settings.dayLightThreshold;
      bool turnOnA = isNight && segA.offset == 0 && segA.period > 0;
      bool turnOnB = isNight && segB.offset == 0 && segB.period > 0;
      bool turnOnC = isNight && segC.offset == 0 && segC.period > 0;
      bool turnOnD = isNight && segD.offset == 0 && segD.period > 0;

      if(prevOnA != turnOnA || prevOnB != turnOnB || prevOnC != turnOnC || prevOnD != turnOnD)
      {
        digitalWrite(LAMP_A, turnOnA ? ON : OFF);
        digitalWrite(LAMP_B, turnOnB ? ON : OFF);
        digitalWrite(LAMP_C, turnOnC ? ON : OFF);
        digitalWrite(LAMP_D, turnOnD ? ON : OFF);

        Serial.printf("Segment A: %s, Segment B: %s, Segment C: %s, , Segment D: %s\r\n", turnOnA ? "ON" : "OFF", turnOnB ? "ON" : "OFF", turnOnC ? "ON" : "OFF", turnOnD ? "ON" : "OFF");
      }

      prevOnA = turnOnA;
      prevOnB = turnOnB;
      prevOnC = turnOnC;
      prevOnD = turnOnD;
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

  pinMode(STATUS_R, OUTPUT);
  pinMode(STATUS_G, OUTPUT);
  pinMode(STATUS_B, OUTPUT);

  pinMode(WIFI_RESET, INPUT);

  pinMode(LAMP_A, OUTPUT);
  pinMode(LAMP_B, OUTPUT);
  pinMode(LAMP_C, OUTPUT);

  digitalWrite(LAMP_A,  OFF);
  digitalWrite(LAMP_B,  OFF);
  digitalWrite(LAMP_C,  OFF);

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
}

void IRAM_ATTR scheduleLampSegmentOn(volatile lampState_t* lampState, unsigned int offset, unsigned int period)
{
  if(lampState->period == 0)
  {
    // Segment is currently off, so set it to turn on for the given period after the given delay.
    lampState->offset = offset;
    lampState->period = period;
  }
  else if(lampState->offset == 0)
  {
    // Segment is currently turned on. Keep it turned on for current period or the given offset+period, whichever is longer
    unsigned int t = lampState->period;
    lampState->period = max (t, offset + period);    
  }
  else
  {
    // The segment is scheduled to be turned on after some delay. Turn it on after that delay or the given offset, whichever is smaller,
    // and keep it on past the scheduled period or the the given offset+period, whichever is larger
    int time_to_turn_off = max(lampState->offset + lampState->period, offset + period);
    unsigned int t = lampState->offset;
    lampState->offset = min(t, offset);
    lampState->period = time_to_turn_off - lampState->offset;
  } 
}

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
