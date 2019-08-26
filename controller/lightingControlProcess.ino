void IRAM_ATTR scheduleLampSegmentOn(volatile lampState_t* lampState, unsigned int offset, unsigned int period);
void IRAM_ATTR turnLampSegmentOn(volatile lampState_t* lampState, unsigned int period);

void IRAM_ATTR onTimer()
{
  portENTER_CRITICAL(&timerMux);
  ++isrCounter;
  portEXIT_CRITICAL(&timerMux);
  
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

void IRAM_ATTR onMotionDEF()
{
  portENTER_CRITICAL(&timerMux);
  sensorTriggerTimestamps.clockD = millis();

  // Turnning on the lamp segment C immediately for the default duration
  turnLampSegmentOn(&lampStateB, settings.regularLampOnTime);
  
  portEXIT_CRITICAL(&timerMux);
}


void lightingControlProcess(void * parameter)
{
  sensorState_t sensorTriggers;
  lampState_tx lampStates;
  int isr_counter;
   
  for(;;)
  {
    if (xSemaphoreTake(timerSemaphore, 0) == pdTRUE)
    {
      // Locking the timer mutex, decrementing and coying positive lamp-state timer values , and releasing the mutex
      portENTER_CRITICAL(&timerMux);
      lampStates.timerA = lampStateTimers.timerA > 0 ? --lampStateTimers.timerA : 0;
      lampStates.timerB = lampStateTimers.timerB > 0 ? --lampStateTimers.timerB : 0;
      lampStates.timerC = lampStateTimers.timerC > 0 ? --lampStateTimers.timerC : 0;

      lampStates.stateA = lampStateTimers.stateA;
      lampStates.stateB = lampStateTimers.stateB;
      lampStates.stateC = lampStateTimers.stateC;
      portEXIT_CRITICAL(&timerMux);

      // Checking the new states of lamp segments depending on whether the corresponding timer values are positive or zero
      bool isNight = digitalRead(DAYLIGHT_SENSOR) != DAYTIME;
      int stateA = isNight && lampStates.timerA > 0;
      int stateB = isNight && lampStates.timerB > 0;
      int stateC = isNight && lampStates.timerC > 0;

      Serial.printf("Is Night: %s", isNight ? "YES" : "NO");
      Serial.println();

      if(stateA != lampStates.stateA || stateB != lampStates.stateB || stateC != lampStates.stateC)
      {
        digitalWrite(LAMP_A, stateA ? ON : OFF);
        digitalWrite(LAMP_B, stateB ? ON : OFF);
        digitalWrite(LAMP_C, stateC ? ON : OFF);

        // The following code which sets the states in lampStateTimers may not needs to be protected with a mutex because these
        // states are only set by the following code segment. However, I protected it anyways.
        portENTER_CRITICAL(&timerMux);
        lampStateTimers.stateA = stateA;
        lampStateTimers.stateB = stateB;
        lampStateTimers.stateC = stateC;       
        portEXIT_CRITICAL(&timerMux);
       
        Serial.printf("Segment A: %s, Segment B: %s, Segment C: %s", stateA ? "ON" : "OFF", stateB ? "ON" : "OFF", stateC ? "ON" : "OFF");
        Serial.println();
      }
    }
    else
      delay(10);
  }
  
}

void initLightingControlSystem()
{
  // Initialzing pin modes
  pinMode(MOTION_A, INPUT_PULLDOWN);
  pinMode(MOTION_A, INPUT_PULLDOWN);
  pinMode(MOTION_B, INPUT_PULLDOWN);
  pinMode(MOTION_C, INPUT_PULLDOWN);
  pinMode(MOTION_D, INPUT_PULLDOWN);
  pinMode(MOTION_E, INPUT_PULLDOWN);
  pinMode(MOTION_F, INPUT_PULLDOWN);
  pinMode(DAYLIGHT_SENSOR, INPUT_PULLDOWN);
  pinMode(WIFI_RESET, INPUT_PULLUP);

  pinMode(LAMP_A, OUTPUT);
  pinMode(LAMP_B, OUTPUT);
  pinMode(LAMP_C, OUTPUT);

  digitalWrite(LAMP_A,  OFF);
  digitalWrite(LAMP_B,  OFF);
  digitalWrite(LAMP_C,  OFF);
  
  // Create semaphore to inform us when the timer has fired
  timerSemaphore = xSemaphoreCreateBinary(); 

  // Use 1st timer of 4 (counted from zero).
  // Set 80 divider for prescaler (see ESP32 Technical Reference Manual for more
  // info).
  timer = timerBegin(0, 80, true);

  // Attach onTimer function to our timer.
  timerAttachInterrupt(timer, &onTimer, true);

  // Set alarm to call onTimer function every second (value in microseconds).
  // Repeat the alarm (third parameter)
  timerAlarmWrite(timer, 1000000, true);

  // Start an alarm
  timerAlarmEnable(timer);

  // Setting up motion-sensor interrupts
  attachInterrupt(MOTION_A, &onMotionA, RISING);
  attachInterrupt(MOTION_B, &onMotionB, RISING);
  attachInterrupt(MOTION_C, &onMotionC, RISING);
  attachInterrupt(MOTION_D, &onMotionDEF, RISING);
  attachInterrupt(MOTION_E, &onMotionDEF, RISING);
  attachInterrupt(MOTION_F, &onMotionDEF, RISING);
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
