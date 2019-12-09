#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

#include "AppConfig.h"

#define SEG_MASK_1 1
#define SEG_MASK_2 2
#define SEG_MASK_3 4
#define SEG_MASK_4 8
#define SEG_MASK_5 16


char *WifiSSID = NULL, *WifiPassword = NULL;

volatile bool FetchScheduleFlag = false;

time_t LastSyncSuccessTime;
time_t LastSyncAttemptTime;
int FailedWifiConnectionAttemptCount;

void systemAdminProcess(void * parameter) {

  // Initializing the lamp-schedule timer
  lampScheduleTimer = timerBegin(1, 80, true);
  timerAttachInterrupt(lampScheduleTimer, &fetchScheduleISR, true);
  timerAlarmWrite(lampScheduleTimer, settings.scheduleCheckInterval * 1000000, true);
  timerAlarmEnable(lampScheduleTimer);
  LastSyncSuccessTime = 0;
  LastSyncAttemptTime = 0;
  FailedWifiConnectionAttemptCount = 0;

  FetchScheduleFlag = true;
  unsigned long lastReportTimestampInSeconds = 0;
  
  for(;;) 
  {
    PrintTime();
    Serial.print(" Admin process looping ... ");
    Serial.printf("Wifi Mode: %s    WiFi Status: %s\r\n", getWifiModeStr(WiFi.getMode()), getWifiStatusStr(wifiStatus));

    if(IsAccessPointPasswordResetRequested())
    {
      Serial.println("Restting access-point password to default value ...");
      
      strncpy(settings.accessPointPassword, DEFAULT_AP_PW, SSID_PASSWORD_MAX_LENGTH-1);
      settings.accessPointPassword[SSID_PASSWORD_MAX_LENGTH-1] = '\0';

      //Saving the updated configuration structure in the EEPROM.
      EEPROM.put(0, settings);
      EEPROM.commit();
      delay(1000);

      // Resetting access point
      Serial.print("Resetting up Access Point …");
      WiFi.softAP(settings.accessPointSsid, settings.accessPointPassword);
      IPAddress IP = WiFi.softAPIP();
      Serial.print(" AP IP address: ");
      Serial.println(IP);
     
      accessPointPasswordResetComplete = true;  
      accessPointPasswordResetBtnPressedTime = 0;
    }
    
    unsigned long t = millis();

    bool trySync = year() == 1970 //Not yest synchronized after the last reset
                      || (now() - LastSyncAttemptTime) > (SYNC_INTERVAL_HOURS * 3600) //synchronization attempt cycle elapsed
                      ;

    //Serial.printf("Elapsed seconds: %d\r\n", now() - LastSyncAttemptTime);

    if(trySync)
    {
      if(WifiConnect())
      {
        bool syncsSucceeded = true;
        FailedWifiConnectionAttemptCount = 0;
  
        syncsSucceeded &= SyncClock();
  
        syncsSucceeded &= SendReport();
  
        syncsSucceeded &= ReloadSchedule(); 

        if(syncsSucceeded)
        {
          LastSyncSuccessTime = now();
        }
      }
      else
      {
        FailedWifiConnectionAttemptCount += 1;
      }
      
      LastSyncAttemptTime = now();
    }    

    WifiDisconnect();
   
    delay(5000);
  }
}

bool SendReport()
{
  int ambientDarkness = 0;
  bool hasActivities = false;
  
  portENTER_CRITICAL(&resourceLock);
  ambientDarkness = darknessLevel;
  hasActivities = gReport.HasActivities();
  portEXIT_CRITICAL(&resourceLock);

  if(hasActivities == false)
    return true; //No need to send a report, so this is not a failure.

  if(WiFi.status() != WL_CONNECTED)
  {
    Serial.println("No wifi connection available");
    return false; //Cannot send the report without having a wifi connection
  }

  WiFiClientSecure client;
  if (!client.connect(LOGGER_URL_HOST, 443)) 
  {
    Serial.println("Report webservice connection failed");
    return false;
  }

  float temperature = getTemperature();
  String params = String("?t=") + temperature + "&d=" + ambientDarkness;

  if(PRODUCTION_MODE)
    params = params + "&mode=prod";
  else
    params = params + "&mode=test&";

  portENTER_CRITICAL(&resourceLock);
  params = params + gReport.Export();
  gReport.Reset();
  portEXIT_CRITICAL(&resourceLock);

  String loggerUrlPath = PRODUCTION_MODE ? PRODUCTION_LOGGER_URL_PATH : TEST_LOGGER_URL_PATH;
  String dataEncodedUrl = loggerUrlPath + params;
  //Serial.println(dataEncodedUrl);
  Serial.println(params);
  client.print(String("GET ") + dataEncodedUrl +" HTTP/1.1\r\n" +
     "Host: " + LOGGER_URL_HOST + "\r\n" + 
     "Connection: close\r\n\r\n");

  //Waiting for the webservice to respond
  String response = "waiting...";
  while (client.connected() && response.length() == 0)
  {
    response = client.readString();
    response.trim();
  }
  
  client.stop();

  return true;
}

///Returns true if the Access-Point-Password-Reset button is kept pressed for 10 seconds
bool IsAccessPointPasswordResetRequested()
{
  unsigned long t = millis();
  while(digitalRead(WIFI_RESET) == false && millis() - t < 10000)
    delay(100);

  return (millis() - t >= 10000)  && (digitalRead(WIFI_RESET) == false);
}

bool WifiConnect()
{  
  if(WiFi.status() == WL_CONNECTED)
  {
    wifiStatus = eWifiStatus::CONNECTION_ACTIVE;
    Serial.println("Wifi connection already exists");
    return true;
  }
  else
  {
//    //make sure we do not connect or re-attempt to connect to wifi unless this is the first attempt
//    //or the time elapsed from the previous connection/connection-reattempt is more than WIFI_REATTEMPT_INTERVAL_MINUTES.
//    unsigned current_time = millis();    
//    unsigned long dt = millisFrom(wifiAttemtTimeStamp) / 60000;
//    if(wifiAttemtTimeStamp > 0 && dt < WIFI_REATTEMPT_INTERVAL_MINUTES)
//    {
//      return false;
//    }

    //If sufficient time has elapsed from the previous attemt, connect to wifi.

    Serial.println("Connecting to wifi ... ");
    wifiAttemtTimeStamp = millis();
    wifiStatus = eWifiStatus::CONNECTION_INPROGRESS;   

    // Turning on WiFi 
    WiFi.mode(WIFI_MODE_APSTA);
    delay(1000);
    
    // Connecting to wifi
    WiFi.begin(settings.wifiSsid, settings.wifiPassword);
      
    //waiting for upto specified number of minutes for a connection
    unsigned int minuts_to_wait = WIFI_TURN_ON_WAIT_TIME_MINUTES;
    unsigned int waited_time = 0;
    while((WiFi.status() != WL_CONNECTED) && (waited_time < minuts_to_wait * 60000))
    {
      delay(5000);
      waited_time = millisFrom(wifiAttemtTimeStamp);
      Serial.printf("Waited time in the loop: %d\r\n", waited_time);
    }

    wifiAttemtTimeStamp = millis();
    if(WiFi.status() == WL_CONNECTED)
    {
      wifiStatus = eWifiStatus::CONNECTION_ACTIVE;
      Serial.print("Wifi connection established. Local IP: ");
      Serial.println(WiFi.localIP());
      return true;
    }
    else
    {
      wifiStatus = eWifiStatus::CONNECTION_FAILED;
      Serial.println("Unnable to connect to wifi");
      return false;
    }
  }  
}

void WifiDisconnect()
{
  WifiDisconnect(false);
}

void WifiDisconnect(bool force)
{
  if(force == false && freshlyRebooted && millis()/60000 < HOTSPOT_ACTIVE_PERIOD_MINUTES)
  {
    //This is a freshly booted cycle, so we should not disconnect wifi right away.
  }
  else
  {
    freshlyRebooted = false;
    if(WiFi.status() == WL_CONNECTED)
    {
      // Indicates that the wifi connection being closed is a successful connection.
      wifiStatus = eWifiStatus::SUCCESS;
    }

    while(WiFi.getMode() != WIFI_OFF)
    {
      Serial.println("Disconnecting wifi");
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      delay(500);
    }
  }
}

const char* getWifiStatusStr(int statusCode)
{
  switch(statusCode)
  {
    case WL_CONNECTED: return "WL_CONNECTED";
    case WL_NO_SHIELD: return "WL_NO_SHIELD";
    case WL_IDLE_STATUS: return "WL_IDLE_STATUS";
    case WL_NO_SSID_AVAIL: return "WL_NO_SSID_AVAIL";
    case WL_SCAN_COMPLETED: return "WL_SCAN_COMPLETED";
    case WL_CONNECT_FAILED: return "WL_CONNECT_FAILED";
    case WL_CONNECTION_LOST: return "WL_CONNECTION_LOST";
    case WL_DISCONNECTED: return "WL_DISCONNECTED";
    default: return "Unknown";
  }
}

const char* getWifiModeStr(int modeCode)
{
  switch(modeCode)
  {
    case WIFI_OFF: return "WIFI_OFF";
    case WIFI_MODE_STA: return "WIFI_MODE_STA";
    case WIFI_MODE_AP: return "WIFI_MODE_AP";
    case WIFI_MODE_APSTA: return "WIFI_MODE_APSTA";
    case WIFI_MODE_MAX: return "WIFI_MODE_MAX";
    default: return "Unknown";
  }
}

bool SyncClock()
{
  if(WiFi.status() == WL_CONNECTED)
  {
    bool success = false;
    WiFiClientSecure client;
    if (!client.connect(TIME_SERVER_HOST, 443)) 
    {
      Serial.println("SyncClock(): Connection to TIME_SERVER_HOST failed");
      return false;
    }

    Serial.println("Synchronizing clock ... ");
    
    client.print(String("GET ") + TIME_SERVER_API +" HTTP/1.1\r\n" +
      "Host: " + TIME_SERVER_HOST + "\r\n" + 
      "Connection: close\r\n\r\n");
     
    while (client.connected()) 
    {
      //Reading the header
      String line;
      do
      {
        line = client.readStringUntil('\n');
        line.trim();
      }
      while(line.length() > 0);
      
      String timeData = client.readString();      
      timeData.trim();
      
      
      //NOTE: Use Arduino Json Assitant to Calculate Document Size
      //      https://arduinojson.org/v5/assistant/
      StaticJsonDocument<TIME_SERVER_RESPONSE_JSON_BUFFER_SIZE> doc;
      DeserializationError error = deserializeJson(doc, timeData);
      if (error) 
      {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
      }
      else
      {
        JsonObject obj = doc.as<JsonObject>();
        time_t unixtime = obj["unixtime"];
        unixtime += (int) obj["raw_offset"];
        setTime(unixtime);        
        Serial.print("Time Data: "); Serial.println(timeData);
        success = true;
      }
    }
    client.stop();
    return success;
  }
  else
  {
    Serial.println("SyncClock(): faild -- WiFi is not connected.");
    return false; 
  }
}

void PrintTime()
{
  Serial.printf("%d-%02d-%02d %02d:%02d:%02d", year(), month(), day(), hour(), minute(), second());
}

unsigned long millisFrom(unsigned long referenceMillis)
{
  unsigned current = millis();    
  unsigned long dt = (current > referenceMillis) ? (current - referenceMillis) : (ULONG_MAX - referenceMillis + current);
  return dt;
}
 
bool ReloadSchedule()
{
  Serial.println("Fetching schedule ...");
  const char* scheduleRetrieverApi = PRODUCTION_MODE ? settings.scheduleApiPath : SCHEDULE_RETRIEVER_TEST_URL;

  unsigned short segments[MAX_SHCEDULED_ITEMS];
  unsigned long offsets[MAX_SHCEDULED_ITEMS];
  unsigned long periods[MAX_SHCEDULED_ITEMS];
  
  int numItemsLoaded = fetchSchedule(SCHEDULE_RETREIAVER_HOST, 443, scheduleRetrieverApi, segments, offsets, periods, MAX_SHCEDULED_ITEMS);
  Serial.printf("Num schedules loaded: %d\r\n", numItemsLoaded);

  if(numItemsLoaded > 0)
  {
    String message = "";
    
    portENTER_CRITICAL(&resourceLock);
    
    gSegmentA.ResetSchedule();
    gSegmentB.ResetSchedule();
    gSegmentC.ResetSchedule();
    gSegmentD.ResetSchedule();
    gSegmentE.ResetSchedule();
    
    for(int i=0; i<numItemsLoaded; ++i)
    {      
      if((segments[i] & SEG_MASK_1) > 0){
        message = message + "\r\nSegment A: " + gSegmentA.ScheduleCycle(offsets[i], periods[i]);
      }
      
      if((segments[i] & SEG_MASK_2) > 0){
        message = message + "\r\nSegment B: " + gSegmentB.ScheduleCycle(offsets[i], periods[i]);
      }

      if((segments[i] & SEG_MASK_3) > 0){
        message = message + "\r\nSegment C: " + gSegmentC.ScheduleCycle(offsets[i], periods[i]);
      }

      if((segments[i] & SEG_MASK_4) > 0){
        message = message + "\r\nSegment D: " + gSegmentD.ScheduleCycle(offsets[i], periods[i]);
      }

      if((segments[i] & SEG_MASK_5) > 0){
        message = message + "\r\nSegment E: " + gSegmentE.ScheduleCycle(offsets[i], periods[i]);
      }          
    }

    portEXIT_CRITICAL(&resourceLock);

    if(message.length() > 0)
    {
      Serial.println(message);
    }
  }

/*   
  return;
 
  if(schedule.length() > 0)
  {
    //NOTE: Use Arduino Json Assitant to Calculate Document Size
    //      https://arduinojson.org/v5/assistant/
    StaticJsonDocument<SCHEDULE_OBJECT_SIZE> doc;
    DeserializationError error = deserializeJson(doc, schedule);
  
    if (error) 
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
    }
    else
    {
      JsonArray arr = doc.as<JsonArray>();
      
      portENTER_CRITICAL(&resourceLock);
      gSegmentA.ResetSchedule();
      gSegmentB.ResetSchedule();
      gSegmentC.ResetSchedule();
      gSegmentD.ResetSchedule();
      gSegmentE.ResetSchedule();
  
      String message = "";
      if(arr.size() > 0)
      {
        
        for(JsonVariant v : arr)
        {
          JsonObject obj = v.as<JsonObject>();
          
          String segments = obj["s"];
          unsigned int offset = obj["offset"];
          unsigned int period = obj["period"];
          
          segments.toUpperCase();
          if(segments.indexOf("S1") >= 0){
            message = message + "\r\nSegment A: " + gSegmentA.ScheduleCycle(offset, period);
          }
          
          if(segments.indexOf("S2") >= 0){
            message = message + "\r\nSegment B: " + gSegmentB.ScheduleCycle(offset, period);
          }
  
          if(segments.indexOf("S3") >= 0){
            message = message + "\r\nSegment C: " + gSegmentC.ScheduleCycle(offset, period);
          }
  
          if(segments.indexOf("S4") >= 0){
            message = message + "\r\nSegment D: " + gSegmentD.ScheduleCycle(offset, period);
          }
  
          if(segments.indexOf("S5") >= 0){
            message = message + "\r\nSegment E: " + gSegmentE.ScheduleCycle(offset, period);
          }          
        }
      }
      portEXIT_CRITICAL(&resourceLock);
  
      if(message.length() > 0)
      {
        Serial.println(message);
      }
    }
  }
  */
}

int fetchSchedule(const char* host, int port, const char* url, unsigned short* segmentsBuffer, unsigned long* offsetsBuffer, unsigned long* periodsBuffer, int maxBufferLength)
{
  WiFiClientSecure client;
  if (!client.connect(host, port)) 
  {
    Serial.println("Connection failed");
    return 0;
  }

  //Serial.printf("%s%s\r\n", host, url);
  
  client.print(String("GET ") + url +" HTTP/1.1\r\n" +
     "Host: " + host + "\r\n" + 
     "Connection: close\r\n\r\n");
       
  bool redirect = false;
  String redirectUrl = "";
  
  while (client.connected()) {

    String line = client.readStringUntil('\n');
   
    if(line.indexOf("HTTP/1.1 ") == 0)
    {
      int pos = line.indexOf(" ", 9);
      int statusCode = line.substring(9, pos).toInt();

      if(statusCode == 301 || statusCode == 302)
      {
        redirect = true;
      }
    }

    if(line.indexOf("Location: ") == 0)
    {
      redirectUrl = line.substring(10);
    }

    if (line == "\r") {
        //Serial.println("headers received");
        break;
      }

  }

  if(redirect && redirectUrl.length() > 0)
  {
    int idx1 = redirectUrl.indexOf("://") + 3;
    int idx2 = redirectUrl.indexOf("/", idx1);
    String host2 = redirectUrl.substring(idx1, idx2);
    String url2 = redirectUrl.substring(idx2);
    
    //Serial.print("Redirecting to host: ");
    //Serial.println(host);
    //Serial.print("Url: ");
    //Serial.println(url);    
    //Serial.println(redirectUrl);

    client.stop();
    return fetchSchedule(host2.c_str(), 443, url2.c_str(), segmentsBuffer, offsetsBuffer, periodsBuffer, maxBufferLength);
  }
  else
  {
    //No more redirections. Refreshing all segments
    String result = "";
    char buff[16];
    Serial.println("Start reading lines ... ");
    int itr = 0;
    while (client.available() && itr < maxBufferLength)
    {
      String line = client.readStringUntil('\n');
      line.trim();

      int idx1 = line.indexOf(":");
      String segments = line.substring(0, idx1);
      segments.toUpperCase();
      
      int idx2 = line.indexOf("|");  
      String offsetStr = line.substring(idx1 + 1, idx2);
      offsetStr.toCharArray(buff, 16);
      unsigned int offset = atoi(buff);

      String periodStr = line.substring(idx2 + 1);
      periodStr.toCharArray(buff, 16);
      unsigned int period = atoi(buff);

      Serial.printf("%s: [%d, %d]\r\n", segments, offset, offset);

      segmentsBuffer[itr] = 0;
      if(segments.indexOf("S1") >=0)
        segmentsBuffer[itr] |= SEG_MASK_1;
      if(segments.indexOf("S2") >=0)
        segmentsBuffer[itr] |= SEG_MASK_2;
      if(segments.indexOf("S3") >=0)
        segmentsBuffer[itr] |= SEG_MASK_3;
      if(segments.indexOf("S4") >=0)
        segmentsBuffer[itr] |= SEG_MASK_4;
      if(segments.indexOf("S5") >=0)
        segmentsBuffer[itr] |= SEG_MASK_5;
      
      offsetsBuffer[itr] = offset;
      periodsBuffer[itr] = period;

      //Serial.printf("segment: %d\r\n", segmentsBuffer[itr]);
           
     ++itr;      
    }
    Serial.println("Done reading lines ... ");

    client.stop();
    
    return itr;
  }
}

String fetchTime()
{
   WiFiClientSecure client;
    const int httpPort = 443;
    if (!client.connect(TIME_SERVER_HOST, httpPort)) 
    {
      Serial.println("Connection failed");
      return "";
    }

    //Serial.println("Fetching current time ...");
  
    client.print(String("GET ") + TIME_SERVER_API +" HTTP/1.1\r\n" +
       "Host: " + TIME_SERVER_HOST + "\r\n" + 
       "Connection: close\r\n\r\n");

    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        //Serial.println("headers received");
        break;
      }
    }

    String result = "";
    while (client.available()) {
      result = result + client.readString();
    }
    client.stop();

    return result;
}


void fetchScheduleISR()
{
  FetchScheduleFlag = true;

}
