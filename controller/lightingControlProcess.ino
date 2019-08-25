
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
  
  lampStateTimers.timerA = settings.lampOnTimeMinutes;
  
  portEXIT_CRITICAL(&timerMux);
}

void IRAM_ATTR onMotionB()
{
  portENTER_CRITICAL(&timerMux);
  sensorTriggerTimestamps.clockB = millis();

  lampStateTimers.timerB = settings.lampOnTimeMinutes;
  
  portEXIT_CRITICAL(&timerMux);
}

void IRAM_ATTR onMotionC()
{
  portENTER_CRITICAL(&timerMux);
  sensorTriggerTimestamps.clockC = millis();
  
  lampStateTimers.timerC = settings.lampOnTimeMinutes;
  
  portEXIT_CRITICAL(&timerMux);
}

void IRAM_ATTR onMotionD()
{
  portENTER_CRITICAL(&timerMux);
  sensorTriggerTimestamps.clockD = millis();

  lampStateTimers.timerA = settings.lampOnTimeMinutes;
  lampStateTimers.timerB = settings.lampOnTimeMinutes;
  
  portEXIT_CRITICAL(&timerMux);
}

void IRAM_ATTR onMotionE()
{
  portENTER_CRITICAL(&timerMux);
  sensorTriggerTimestamps.clockE = millis();

  lampStateTimers.timerA = settings.lampOnTimeMinutes;
  lampStateTimers.timerB = settings.lampOnTimeMinutes;
  lampStateTimers.timerC = settings.lampOnTimeMinutes;

  portEXIT_CRITICAL(&timerMux);
}

void initLightingControlSystem()
{
  // Initialzing pin modes
  pinMode(MOTION_A, INPUT);
  pinMode(MOTION_B, INPUT);
  pinMode(MOTION_C, INPUT);
  pinMode(MOTION_D, INPUT);
  pinMode(MOTION_E, INPUT);
  pinMode(DAYLIGHT_SENSOR, INPUT_PULLUP);
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
  attachInterrupt(MOTION_D, &onMotionD, RISING);
  attachInterrupt(MOTION_E, &onMotionE, RISING);
}

void lightingControlProcess(void * parameter)
{
  sensorState_t sensorTriggers;
  lampState_t lampStates;
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
        //digitalWrite(LAMP_C, stateC ? ON : OFF);
        digitalWrite(LAMP_C, stateC ? OFF : ON);

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
