#ifndef _MOTION_SENSOR_H
#define _MOTION_SENSOR_H

#define TRIGGER_BUFFER_SIZE 50

class MotionSensor
{
  private:
  unsigned int mInputPin;

  volatile unsigned long mTriggers[TRIGGER_BUFFER_SIZE];
  volatile unsigned int mIndex;
  volatile int mBufferOverrunCount;

  public:
  MotionSensor(unsigned int inputPin)
  {
    mInputPin = inputPin;
    mIndex = 0;
    mBufferOverrunCount = 0;
  }

  void Trigger() volatile
  {
    if(mIndex == TRIGGER_BUFFER_SIZE - 1)
    {
      ++mBufferOverrunCount;
    }
    else
    {
      mTriggers[mIndex++] = millis();
      mBufferOverrunCount = 0;
    }
  }



};

#endif
