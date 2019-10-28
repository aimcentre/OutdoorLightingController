#ifndef _REPORT_H
#define _REPORT_H

#define REPORT_BUFFER_SIZE 50

class Report
{
  public:
  enum eAction{ NONE = 0, MSA_TRIGGER = 1, MSB_TRIGGER, MSC_TRIGGER, MSD_TRIGGER, MSE_TRIGGER, MSF_TRIGGER, MSG_TRIGGER,
              L1_ON, L1_OFF, L2_ON, L2_OFF, L3_ON, L3_OFF, L4_ON, L4_OFF, L5_ON, L5_OFF };

  private:
  volatile eAction mAction[REPORT_BUFFER_SIZE];
  volatile unsigned long mTimestamps[REPORT_BUFFER_SIZE];
  volatile unsigned int mIndex;

  public:
  static Report Instance;
  
  Report()
  {
    mIndex = 0;
  }

  void AddAction(eAction action) volatile
  {
    int idx = mIndex < REPORT_BUFFER_SIZE - 1 ? mIndex++ : REPORT_BUFFER_SIZE - 1;

    mAction[idx] = action;
    mTimestamps[idx] = millis();
    //Serial.printf("Idx =  %d\r\n", idx);    
  }

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
};


#endif
