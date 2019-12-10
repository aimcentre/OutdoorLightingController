#ifndef _REPORT_H
#define _REPORT_H

#include <time.h>
#include "AppConfig.h"

//#define ACTION_HISTORY_LENGTH 10
#define ACTION_HISTORY_BUFFER_SIZE 1000
#define SEPARATOR "_"

/*
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
*/

class Report
{
  public:
  enum eAction{ NONE = 0, MSA_TRIGGER = 1, MSB_TRIGGER, MSC_TRIGGER, MSD_TRIGGER, MSE_TRIGGER, MSF_TRIGGER, MSG_TRIGGER,
              L1_ON, L1_OFF, L2_ON, L2_OFF, L3_ON, L3_OFF, L4_ON, L4_OFF, L5_ON, L5_OFF,
              PING,
              NUM_TOKENS};

  private:
  //volatile ActionHistory ActionHistories[NUM_TOKENS];

  volatile unsigned int Index;
  volatile time_t Timestamps[ACTION_HISTORY_BUFFER_SIZE];
  volatile eAction Actions[ACTION_HISTORY_BUFFER_SIZE];

  public:
  static Report Instance;
  
  Report()
  {
    Reset();
  }

  int AddAction(eAction action) volatile
  {
//    ActionHistories[action].LogOccurance();

    if(Index < ACTION_HISTORY_BUFFER_SIZE)
    {
      Timestamps[Index] = now();
      Actions[Index] = action;
      ++Index;

      Serial.printf("%d: %s\r\n", Index, GetActionString(action));
    }
    else
      Serial.printf("SKIP: %s\r\n", GetActionString(action));
    
    return Index;
  }

  void Reset() volatile
  {
    Serial.println("Resetting report");
    Index = 0;
    
//    for(int i=0; i<NUM_TOKENS; ++i)
//      ActionHistories[i].Reset();
  }

  bool HasActivities() volatile
  {
    return Index > 0;
//    for(int i=0; i<NUM_TOKENS; ++i)
//    if(ActionHistories[i].Count > 0)
//      return true;
//
//    return false;
  }

  String Export() volatile
  {
    String ret = "";
/*
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
  */
    return ret;
  }

  const char* GetActionString(eAction action) volatile
  {
    switch(action)
    {
      case NONE: return "NONE"; break;
      case MSA_TRIGGER: return "MSA_TRIGGER"; break;
      case MSB_TRIGGER: return "MSB_TRIGGER"; break;
      case MSC_TRIGGER: return "MSC_TRIGGER"; break;
      case MSD_TRIGGER: return "MSD_TRIGGER"; break;
      case MSE_TRIGGER: return "MSE_TRIGGER"; break;
      case MSF_TRIGGER: return "MSF_TRIGGER"; break;
      case MSG_TRIGGER: return "MSG_TRIGGER"; break;
      case L1_ON:  return "L1_ON"; break;
      case L1_OFF: return "L1_OFF"; break;
      case L2_ON:  return "L2_ON"; break;
      case L2_OFF: return "L2_OFF"; break;
      case L3_ON:  return "L3_ON"; break;
      case L3_OFF: return "L3_OFF"; break;
      case L4_ON:  return "L4_ON"; break;
      case L4_OFF: return "L4_OFF"; break;
      case L5_ON:  return "L5_ON"; break;
      case L5_OFF: return "L5_OFF"; break;
      case PING:   return "PING"; break;
    }
  }
};


#endif
