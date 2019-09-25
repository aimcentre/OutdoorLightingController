// Lamp on/off output level 
#define ON HIGH
#define OFF LOW

#include "systemConfigParams.h"

class LampSegment
{
  private:
  unsigned int mOffset;
  unsigned int mPeriod;
  unsigned int mOutputPin;
  unsigned int mAmbientDarkness;
  bool mLampStatus;
  bool mPrevLampStatus;

  public:
  LampSegment(unsigned int outputPin)
  {
    mOutputPin = outputPin;
    mOffset = 0;
    mPeriod = 0;
    mAmbientDarkness - 0;
    mLampStatus = OFF;
    mPrevLampStatus = OFF;
  }

  void Update(unsigned int offset, unsigned int period) volatile
  {    
    if(mPeriod == 0)
    {
      // Segment is currently off, so set it to turn on for the given period after the given delay.
      mOffset = offset;
      mPeriod = period;
    }
    else
    {
      // The segment is either currently on or it is scheduled to be on
      
      if(mOffset == 0)
      {
        // Segment is currently turned on. Keep it turned on for current period or the given offset+period, whichever is longer
        unsigned t = offset + period;
        if(t > mPeriod)
          mPeriod = t;
      }
      else
      {
        // The segment is scheduled to be turned on after the delay specified by mOffset. 
        // Turn it on after that delay or the given offset, whichever is smaller,
        // and then keep it on past the scheduled period or the the given offset+period, whichever is larger
        int time_to_turn_off = max(mOffset + mPeriod, offset + period);
        if(offset < mOffset)
          mOffset = offset;
        mPeriod = time_to_turn_off - mOffset;
      }
    }

    Serial.printf("Lamp %d: Offset = %d, Period = %d\r\n", this, mOffset, mPeriod);
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
    if(mOffset > 0)
      --mOffset;

    if(mOffset == 0 && mPeriod > 0)
      --mPeriod;

    mAmbientDarkness = ambientDarkness;
    mLampStatus = (ambientDarkness > darknessThreshold && mOffset == 0 && mPeriod > 0) ? ON : OFF;

    Serial.printf("OnTick() called; Offset: %d, Period: %d, Lamp Status = %s\r\n", mOffset, mPeriod, mLampStatus == ON ? "ON" : "OFF");
  }

  void Reset() volatile
  {
    mOffset = 0;
    mPeriod = 0;
  }
  
};
