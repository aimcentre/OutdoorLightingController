#ifndef _LAMP_SEGMENT_H
#define _LAMP_SEGMENT_H

#include "systemConfigParams.h"
#include "LampCycle.h"
#include "Report.h"

// Lamp on/off output level 
#define ON HIGH
#define OFF LOW

#define LAMP_CYCLE_BUFFER_SIZE 50

class LampSegment
{
  private:
  unsigned int mOffset;
  unsigned int mPeriod;
  unsigned int mOutputPin;
  unsigned int mAmbientDarkness;
  bool mLampStatus;
  bool mPrevLampStatus;
  
  volatile LampCycle mLampCycleList[LAMP_CYCLE_BUFFER_SIZE];

  public:
  LampSegment(unsigned int outputPin)
  {
    mOutputPin = outputPin;
    mAmbientDarkness = 0;
    mLampStatus = OFF;
    mPrevLampStatus = OFF;
  }

  public:

  /// UpdateLampCycle: Updates the offset and the period of the LampCycle at the given index of mLampCycleList
  ///   If the status of the LampCycle is "done", then sets the given offset and period to it. Otherwise, adjust its 
  ///   offset and period such that the resulting offset and the period will cover both current and new values 
  ///    of these parameters.
  void UpdateLampCycle(int lampCycleIndex, unsigned int offset, unsigned int period) volatile
  {
    if(mLampCycleList[lampCycleIndex].GetStatus() == LampCycle::eCycleState::DONE)
    {
      mLampCycleList[lampCycleIndex].mOffset = offset;
      mLampCycleList[lampCycleIndex].mPeriod = period;
    }
    else
    {
      int time_to_turn_on = mLampCycleList[lampCycleIndex].mOffset < offset ? mLampCycleList[lampCycleIndex].mOffset : offset;
      int time_to_turn_off = max(mLampCycleList[lampCycleIndex].mOffset + mLampCycleList[lampCycleIndex].mPeriod, offset + period);
      mLampCycleList[lampCycleIndex].mOffset = time_to_turn_on;
      mLampCycleList[lampCycleIndex].mPeriod = time_to_turn_off - time_to_turn_on;
    }
  }

  /// Sets the offset and period of the default LampCycle object of this segment
  void Trigger(unsigned int offset, unsigned int period) volatile
  {
    UpdateLampCycle(0, offset, period);
  }
  
  /// ScheduleCycle: Updates the pLampCycleList by either modifying the timing of an existing LampCycle object
  /// in the list or by inserting a new LampCycle object. This method makes sure that the segment is turned on
  /// from the given time period starting from the given time offset 
  String ScheduleCycle(unsigned int offset, unsigned int period) volatile
  {
    int scheduledCycleStartIndex = 1; //We reserve the cycle 0 to on-site triggers, so scheduled cycles are made from index 1 in the lamp-cycle buffer
    
    //Finding a LampCycle which has an overlapping "on" time with the given period starting from the given offset.
    int targetLampCycleIndex = -1;
    for(int i=scheduledCycleStartIndex; i<LAMP_CYCLE_BUFFER_SIZE; ++i)
    {
      if(mLampCycleList[i].mOffset <= offset && (mLampCycleList[i].mOffset + mLampCycleList[i].mPeriod) >= offset)
      {
        targetLampCycleIndex = i;
        break;
      }
    }

    if(targetLampCycleIndex == -1)
    {
      //No overlapping cycle found, so update the first LampCycle which is already done, or the last one in the buffer if nothing is done.
      for(int i=scheduledCycleStartIndex; i<LAMP_CYCLE_BUFFER_SIZE; ++i)
      {
        if(mLampCycleList[i].GetStatus() == LampCycle::eCycleState::DONE)
        {
          targetLampCycleIndex = i;
          break;
        }
      }
    }

    if(targetLampCycleIndex == -1)
    {
      return "Lamp-cycle schedule is full. Ignoring the scheduling request";
    }
    else
    {
      UpdateLampCycle(targetLampCycleIndex, offset, period);  
      return "Scheduled lamp cycle at index " + String(targetLampCycleIndex); 
    }     
  }
  
  void OnTick(unsigned int ambientDarkness, unsigned int darknessThreshold) volatile
  {     
    //Invoking OnTick() of eachLampCycle to update their offsets and periods as necessary. 
    // Also checking whether there is at least one active LampCycle exists after updating
    bool activeCycleFound = false;
    for(int i=0; i<LAMP_CYCLE_BUFFER_SIZE; ++i)    
    {
      mLampCycleList[i].OnTick();

      if(activeCycleFound == false && mLampCycleList[i].GetStatus() == LampCycle::eCycleState::ACTIVE)
        activeCycleFound = true;
    }

    mAmbientDarkness = ambientDarkness;
    mLampStatus = (ambientDarkness > darknessThreshold && activeCycleFound) ? ON : OFF;
  }

  bool Execute() volatile
  {
    if(mPrevLampStatus != mLampStatus)
    {
      digitalWrite(mOutputPin, mLampStatus);
      mPrevLampStatus = mLampStatus;

      return true;
    }
    
    return false;    
  }

  unsigned int GetStatus() volatile
  {
    return mLampStatus;
  }

  void ResetAll() volatile
  {
    //Clearing the all LampCycle objects in the buffer
    for(int i=0; i<LAMP_CYCLE_BUFFER_SIZE; ++i)
      mLampCycleList[i].Reset();
  }

  void ResetSchedule() volatile
  {
    //Clearing the all LampCycle objects in the buffer
    for(int i=1; i<LAMP_CYCLE_BUFFER_SIZE; ++i)
      mLampCycleList[i].Reset();
    
  }
  
};

#endif
