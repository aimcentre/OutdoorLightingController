#ifndef _LAMP_CYCLE_H
#define _LAMP_CYCLE_H

class LampCycle
{
  public:
  enum eCycleState{ ACTIVE = 1, PENDING = 2, DONE = 3};

  unsigned int mOffset;
  unsigned int mPeriod;

  LampCycle()
  {
    mOffset = 0;
    mPeriod = 0;
  }

  LampCycle(unsigned int offset, unsigned int period)
  {
    mOffset = offset;
    mPeriod = period;
  }
  
  void OnTick() volatile
  {
    if(mOffset > 0)
      --mOffset;
    else if(mPeriod > 0)
      --mPeriod;
  }

  eCycleState GetStatus() volatile
  {
    if(mOffset == 0 && mPeriod > 0)
      return eCycleState::ACTIVE;
    else if(mOffset > 0 && mPeriod > 0)
      return eCycleState::PENDING;
    else
      return eCycleState::DONE;
  }

  void Reset() volatile
  {
    mOffset = 0;
    mPeriod = 0;
  }
};

#endif
