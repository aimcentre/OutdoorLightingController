#ifndef _SUN_LIGHT_SENSOR_H
#define _SUN_LIGHT_SENSOR_H

#define SUNLIGHT_SENSOR_BUFFER_SIZE 50

class SunlightSensor
{
  private:
  volatile int mInputPin;
  volatile int mDarknessThresholdHigh;
  volatile int mDarknessThresholdLow;
  volatile int mDarknessLevelBuffer[SUNLIGHT_SENSOR_BUFFER_SIZE];
  volatile int mBufferIndex;
  volatile int mDarknessLevel;
  volatile bool mIsNight;
  
  public:
  SunlightSensor()
  {
  }

  void Initialize(int inputPin, int darknessThresholdHigh, int darknessThresholdLow)
  {
    mInputPin = inputPin;
    mBufferIndex = 0;
    mDarknessLevel;
    mDarknessThresholdHigh = darknessThresholdHigh;
    mDarknessThresholdLow = darknessThresholdLow;
    mIsNight = false;

    for(int i=0; i<SUNLIGHT_SENSOR_BUFFER_SIZE; ++i)
      mDarknessLevelBuffer[i] = -1;    
  }

  void OnTick() volatile
  {
    // Enforcing a circular buffer
    if(mBufferIndex == SUNLIGHT_SENSOR_BUFFER_SIZE)
      mBufferIndex = 0;

    // Reading the light intensity to the next buffer index
    mDarknessLevelBuffer[mBufferIndex++] = (int) analogRead(mInputPin);

    // Calculating the average light level
    mDarknessLevel = 0;
    int count = 0;
    for(int i=0; i<SUNLIGHT_SENSOR_BUFFER_SIZE; ++i)
    {
      if(mDarknessLevelBuffer[i] >= 0)
      {
        mDarknessLevel = mDarknessLevel + mDarknessLevelBuffer[i];
        ++count;
      }
      //Serial.printf("D[%d] = %d\r\n", i , mDarknessLevelBuffer[i]);
    }
    mDarknessLevel = mDarknessLevel / count;

    // Hysteresis based thresholding
    if(mIsNight)
    {
      if(mDarknessLevel < mDarknessThresholdLow)
        mIsNight = false;
    }
    else
    {
      if(mDarknessLevel >= mDarknessThresholdHigh)
        mIsNight = true;
    }

/*
    // Forcing the Sunlight sensor to enable night mode until the buffer has at least 5 readings
    if(count <= 5)
    {
      mIsNight = true;
    }
    else
    {
      if(mIsNight)
      {
        if(mDarknessLevel < mDarknessThresholdLow)
          mIsNight = false;
      }
      else
      {
        if(mDarknessLevel > mDarknessThresholdHigh)
          mIsNight = true;
      }
    }
*/
    //Serial.printf("Darkness Level = %d, High = %d, Low = %d\r\n", mDarknessLevel, mDarknessThresholdHigh, mDarknessThresholdLow);
    //Serial.printf("Is Night = %d\r\n", mIsNight);
  }

  bool IsNight()
  {
    return mIsNight;
  }

  int getDarknessLevel()
  {
    return mDarknessLevel;
  }
};



#endif
