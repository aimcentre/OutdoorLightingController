#ifndef _REPORT_H
#define _REPORT_H

#include "AppConfig.h"

#define ACTION_HISTORY_LENGTH 10
#define SEPARATOR "_"

class ActionHistory
{
  public:
  volatile int Count;
  volatile unsigned long Timestamps[ACTION_HISTORY_LENGTH];
  volatile unsigned long LastOccuranceTimestamp;

  ActionHistory()
  {
    Reset();
    LastOccuranceTimestamp = 0;
  }

  void Reset() volatile
  {
    Count = 0;
  }

  void LogOccurance() volatile
  {
    unsigned int now = millis();
    unsigned int duration = (LastOccuranceTimestamp < now) ? (LastOccuranceTimestamp - now) : (ULONG_MAX - LastOccuranceTimestamp + now);
    if(duration > ACTION_JITTER_PERIOD_MILLISEC)
    {
      ++Count;
      
      //Serial.println("Logging: ");
      if(Count < ACTION_HISTORY_LENGTH)
        Timestamps[Count] = now;
        
      LastOccuranceTimestamp = now;
    }
  }

  String Export() volatile
  {
    //Serial.print("Exporting: "); Serial.println(Count);
    
    if(Count == 0)
      return "";

    String ret = String(Count) + ":";
    int maxTimestamps = Count < ACTION_HISTORY_LENGTH ? Count : ACTION_HISTORY_LENGTH;
    for(int i=0; i<maxTimestamps; ++i)
    {
      ret = ret + String(Timestamps[i]);
      if(i < (maxTimestamps-1))
        ret = ret + ",";
    }
    
    return ret;
  }
};

class Report
{
  public:
  enum eAction{ NONE = 0, MSA_TRIGGER = 1, MSB_TRIGGER, MSC_TRIGGER, MSD_TRIGGER, MSE_TRIGGER, MSF_TRIGGER, MSG_TRIGGER,
              L1_ON, L1_OFF, L2_ON, L2_OFF, L3_ON, L3_OFF, L4_ON, L4_OFF, L5_ON, L5_OFF,
              PING,
              NUM_TOKENS};

  private:
  volatile ActionHistory ActionHistories[NUM_TOKENS];

  public:
  static Report Instance;
  
  Report()
  {
    Reset();
  }

  void AddAction(eAction action) volatile
  {
    ActionHistories[action].LogOccurance();
  }

  void Reset() volatile
  {
    Serial.println("Resetting report");
    for(int i=0; i<NUM_TOKENS; ++i)
      ActionHistories[i].Reset();
  }

  bool HasActivities() volatile
  {    
    for(int i=0; i<NUM_TOKENS; ++i)
    if(ActionHistories[i].Count > 0)
      return true;

    return false;
  }

  String Export() volatile
  {
    String ret = "";

    if(ActionHistories[MSA_TRIGGER].Count > 0)
      ret = ret + "&mA=" + ActionHistories[MSA_TRIGGER].Export();

    if(ActionHistories[MSB_TRIGGER].Count > 0)
      ret = ret + "&mB=" + ActionHistories[MSB_TRIGGER].Export();

    if(ActionHistories[MSC_TRIGGER].Count > 0)
      ret = ret + "&mC=" + ActionHistories[MSC_TRIGGER].Export();

    if(ActionHistories[MSD_TRIGGER].Count > 0)
      ret = ret + "&mD=" + ActionHistories[MSD_TRIGGER].Export();

    if(ActionHistories[MSE_TRIGGER].Count > 0)
      ret = ret + "&mE=" + ActionHistories[MSE_TRIGGER].Export();
    
    if(ActionHistories[MSF_TRIGGER].Count > 0)
      ret = ret + "&mF=" + ActionHistories[MSF_TRIGGER].Export();

    if(ActionHistories[MSG_TRIGGER].Count > 0)
      ret = ret + "&mG=" + ActionHistories[MSG_TRIGGER].Export();

      
    if(ActionHistories[L1_ON].Count > 0)
      ret = ret + "&s1=1:" + ActionHistories[L1_ON].Export();
    
    if(ActionHistories[L1_OFF].Count > 0)
      ret = ret + (ActionHistories[L1_ON].Count > 0 ? SEPARATOR : "&s1=") + "0:" + ActionHistories[L1_OFF].Export();
  
    if(ActionHistories[L2_ON].Count > 0)
      ret = ret + "&s2=1:" + ActionHistories[L2_ON].Export();
    
    if(ActionHistories[L2_OFF].Count > 0)
       ret = ret + (ActionHistories[L2_ON].Count > 0 ? SEPARATOR : "&s2=") + "0:" + ActionHistories[L2_OFF].Export();
  
    if(ActionHistories[L3_ON].Count > 0)
      ret = ret + "&s3=1:" + ActionHistories[L3_ON].Export();
    
     if(ActionHistories[L3_OFF].Count > 0)
      ret = ret + (ActionHistories[L3_ON].Count > 0 ? SEPARATOR : "&s3=") + "0:" + ActionHistories[L3_OFF].Export();
  
    if(ActionHistories[L4_ON].Count > 0)
      ret = ret + "&s4=1:" + ActionHistories[L4_ON].Export();
    
     if(ActionHistories[L4_OFF].Count > 0)
      ret = ret + (ActionHistories[L4_ON].Count > 0 ? SEPARATOR : "&s4=") + "0:" + ActionHistories[L4_OFF].Export();
  
    return ret;
  }
};


#endif
