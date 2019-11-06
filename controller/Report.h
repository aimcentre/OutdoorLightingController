#ifndef _REPORT_H
#define _REPORT_H

//#define REPORT_BUFFER_SIZE 50
#define ACTION_HISTORY_LENGTH 10

class ActionHistory
{
  public:
  volatile int Count;
  volatile unsigned long Timestamps[ACTION_HISTORY_LENGTH];

  ActionHistory()
  {
    Reset();
  }

  void Reset() volatile
  {
    Count = 0;
  }

  void LogOccurance() volatile
  {
    //Serial.println("Logging: ");
    if(Count < ACTION_HISTORY_LENGTH)
      Timestamps[Count] = millis();
    ++Count;
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
//  volatile eAction mAction[REPORT_BUFFER_SIZE];
//  volatile unsigned long mTimestamps[REPORT_BUFFER_SIZE];
//  volatile unsigned int mIndex;

  public:
  volatile ActionHistory ActionHistories[NUM_TOKENS];

  public:
  static Report Instance;
  
  Report()
  {
 //   mIndex = 0;
    Reset();
  }

  void AddAction(eAction action) volatile
  {
    /*
    int idx = mIndex < REPORT_BUFFER_SIZE - 1 ? mIndex++ : REPORT_BUFFER_SIZE - 1;

    mAction[idx] = action;
    mTimestamps[idx] = millis();
    //Serial.printf("Idx =  %d\r\n", idx);
    */

    //Serial.print("Added action: "); Serial.println(action);
    ActionHistories[action].LogOccurance();
    //Serial.print("Count: "); Serial.println(ActionHistories[action].Count);
  }

  void Reset() volatile
  {
    Serial.println("Resetting report");
    for(int i=0; i<NUM_TOKENS; ++i)
      ActionHistories[i].Reset();
  }

/*  int GetActionCount() volatile
  {
    return mIndex;
  }
*/

  bool HasActivities() volatile
  {    
    for(int i=0; i<NUM_TOKENS; ++i)
    if(ActionHistories[i].Count > 0)
      return true;

    return false;
  }

  String Export() volatile
  {
    Serial.println("L1_ON ... " + String(ActionHistories[L1_ON].Count));

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
      ret = ret + ":0:" + ActionHistories[L1_OFF].Export();
  
    if(ActionHistories[L2_ON].Count > 0)
      ret = ret + "&s2=1:" + ActionHistories[L2_ON].Export();
    
     if(ActionHistories[L2_OFF].Count > 0)
      ret = ret + ":0:" + ActionHistories[L2_OFF].Export();
  
    if(ActionHistories[L3_ON].Count > 0)
      ret = ret + "&s3=1:" + ActionHistories[L3_ON].Export();
    
     if(ActionHistories[L3_OFF].Count > 0)
      ret = ret + ":0:" + ActionHistories[L3_OFF].Export();
  
    if(ActionHistories[L4_ON].Count > 0)
      ret = ret + "&s4=1:" + ActionHistories[L4_ON].Export();
    
     if(ActionHistories[L4_OFF].Count > 0)
      ret = ret + ":0:" + ActionHistories[L4_OFF].Export();
  
    return ret;
/*    
    return ActionHistories[MSA_TRIGGER].Count > 0 ? ("mA=" + ActionHistories[MSA_TRIGGER].Export()) : "" +
           ActionHistories[MSB_TRIGGER].Count > 0 ? ("&mB=" + ActionHistories[MSB_TRIGGER].Export()) : "" +
           ActionHistories[MSC_TRIGGER].Count > 0 ? ("&mC=" + ActionHistories[MSC_TRIGGER].Export()) : "" +
           ActionHistories[MSD_TRIGGER].Count > 0 ? ("&mD=" + ActionHistories[MSD_TRIGGER].Export()) : "" +
           ActionHistories[MSE_TRIGGER].Count > 0 ? ("&mE=" + ActionHistories[MSE_TRIGGER].Export()) : "" +
           ActionHistories[MSF_TRIGGER].Count > 0 ? ("&mF=" + ActionHistories[MSF_TRIGGER].Export()) : "" +
           ActionHistories[MSG_TRIGGER].Count > 0 ? ("&mG=" + ActionHistories[MSG_TRIGGER].Export()) : "" +
           
           ActionHistories[L1_ON].Count > 0 ? ("&s1=1" + ActionHistories[eAction::L1_ON].Export()) : "" +
 //          ActionHistories[L1_OFF].Count > 0 ? (";0:" + ActionHistories[eAction::L1_OFF].Export()) : "" +

           ActionHistories[L2_ON].Count > 0 ? ("&s2=1:" + ActionHistories[eAction::L2_ON].Export()) : "" +
 //          ActionHistories[L2_OFF].Count > 0 ? (";0:" + ActionHistories[eAction::L2_OFF].Export()) : "" +

           ActionHistories[L3_ON].Count > 0 ? ("&s3=1:" + ActionHistories[L3_ON].Export()) : "" +
 //          ActionHistories[L3_OFF].Count > 0 ? (";0:" + ActionHistories[L3_OFF].Export()) : "" +

           ActionHistories[L4_ON].Count > 0 ? ("&s4=1:" + ActionHistories[L4_ON].Export()) : "" +
 //          ActionHistories[L4_OFF].Count > 0 ? (";0:" + ActionHistories[L4_OFF].Export()) : "" +

           ActionHistories[L5_ON].Count > 0 ? ("&s5=1:" + ActionHistories[L5_ON].Export()) : "" 
 //          ActionHistories[L5_OFF].Count > 0 ? ("'0:" + ActionHistories[L5_OFF].Export()) : ""
           ;
           */
  }
/*  
  int ExportTriggers(eAction* actionBuffer, unsigned long* timestampBuffer) volatile
  {
    // Copying data to the destination buffer
    int count = mIndex < REPORT_BUFFER_SIZE ? mIndex : REPORT_BUFFER_SIZE;
    for(int i=0; i<count; ++i)
    {
      *(actionBuffer + i) = mAction[i];
      *(timestampBuffer + i) = mTimestamps[i];
    }

    // Resetting the index
    mIndex = 0;

    // Returning the number of trigers coppied
    return count;
  }
*/
};


#endif
