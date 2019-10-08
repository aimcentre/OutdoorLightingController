#ifndef _LAMP_SEGMENT_H
#define _LAMP_SEGMENT_H

#include "systemConfigParams.h"
#include "LampCycle.h"

// Lamp on/off output level 
#define ON HIGH
#define OFF LOW

#define BUFFER_SIZE 50

class LampSegment
{
  private:
  unsigned int mOffset;
  unsigned int mPeriod;
  unsigned int mOutputPin;
  unsigned int mAmbientDarkness;
  bool mLampStatus;
  bool mPrevLampStatus;

  LampCycle mLampCycleList[BUFFER_SIZE];

  public:
  LampSegment(unsigned int outputPin)
  {
    mOutputPin = outputPin;
    mAmbientDarkness = 0;
    mLampStatus = OFF;
    mPrevLampStatus = OFF;
  }

  public:
  /// ScheduleCycle: Updates the pLampCycleList by either modifying the timing of an existing LampCycle object
  /// in the list or by inserting a new LampCycle object. This method makes sure that the segment is turned on
  /// from the given time period starting from the given time offset 
  void ScheduleCycle(unsigned int offset, unsigned int period) volatile
  {
    //Finding a LampCycle which has an overlapping "on" time with the given period starting from the given offset.
    volatile LampCycle* targetLampCycle = 0;
    
    for(int i=0; i<BUFFER_SIZE; ++i)
    {
      if(mLampCycleList[i].mOffset <= offset && (mLampCycleList[i].mOffset + mLampCycleList[i].mPeriod) >= offset)
      {
        targetLampCycle = &mLampCycleList[i];
        break;
      }
    }

    if(targetLampCycle == 0)
    {
      //No overlapping cycle found, so update the first LamCycle which is already done, or the last one in the buffer if nothing is done.
      for(int i=0; i<BUFFER_SIZE; ++i)
      {
        if(mLampCycleList[i].GetStatus() == LampCycle::eCycleState::DONE)
        {
          targetLampCycle = &mLampCycleList[i];
          break;
        }

        if(targetLampCycle == 0)
        {
          targetLampCycle = &mLampCycleList[BUFFER_SIZE - 1];
        }
      }
      targetLampCycle->mOffset = offset;
      targetLampCycle->mPeriod = period;
      //Serial.println("Added new cycle");
      
    }
    else
    {
      //Serial.println("Updating existing cycle");
      
      //Overlapping cycle found. Adjust it's timing to make sure the cycle covers the given period from the given offset
      int time_to_turn_on = targetLampCycle->mOffset < offset ? targetLampCycle->mOffset : offset;
      int time_to_turn_off = max(targetLampCycle->mOffset + targetLampCycle->mPeriod, offset + period);
      targetLampCycle->mOffset = time_to_turn_on;
      targetLampCycle->mPeriod = time_to_turn_off - time_to_turn_on;
    }
    
  }
  
  void Execute() volatile
  {
    if(mPrevLampStatus != mLampStatus)
    {
      digitalWrite(mOutputPin, mLampStatus);
      mPrevLampStatus = mLampStatus;
    }
    
  }

  void OnTick(unsigned int ambientDarkness, unsigned int darknessThreshold) volatile
  {     
    //Invoking OnTick() of eachLampCycle to update their offsets and periods as necessary. 
    // Also checking whether there is at least one active LampCycle exists after updating
    bool activeCycleFound = false;
    for(int i=0; i<BUFFER_SIZE; ++i)    
    {
      mLampCycleList[i].OnTick();

      if(activeCycleFound == false && mLampCycleList[i].GetStatus() == LampCycle::eCycleState::ACTIVE)
        activeCycleFound = true;
    }

    mAmbientDarkness = ambientDarkness;
    mLampStatus = (ambientDarkness > darknessThreshold && activeCycleFound) ? ON : OFF;
  }

  void Reset() volatile
  {
    //Clearing the all LampCycle objects in the buffer
    for(int i=0; i<BUFFER_SIZE; ++i)
    {
      mLampCycleList[i].mOffset= 0;
      mLampCycleList[i].mPeriod= 0;
    }
  }
  
};

#endif
