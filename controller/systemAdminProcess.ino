#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "AppConfig.h"

char *WifiSSID = NULL, *WifiPassword = NULL;

volatile bool FetchScheduleFlag = false;

void systemAdminProcess(void * parameter) {

  // Initializing the lamp-schedule timer
  lampScheduleTimer = timerBegin(1, 80, true);
  timerAttachInterrupt(lampScheduleTimer, &fetchScheduleISR, true);
  timerAlarmWrite(lampScheduleTimer, settings.scheduleCheckInterval * 1000000, true);
  timerAlarmEnable(lampScheduleTimer);

  FetchScheduleFlag = true;
  unsigned long lastReportTimestampInSeconds = 0;
  String loggerUrlPath = PRODUCTION_MODE ? PRODUCTION_LOGGER_URL_PATH : TEST_LOGGER_URL_PATH;
  for(;;) 
  {

    // Checking if Access Point password-reset button is kept pressed (in which case it retrnse false) false for10 second
    unsigned long t = millis();
    while(digitalRead(WIFI_RESET) == false && millis() - t < 10000)
      delay(100);

    if(millis() - t >= 10000  && digitalRead(WIFI_RESET) == false)
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
      Serial.print("AP IP address: ");
      Serial.println(IP);
     
      accessPointPasswordResetComplete = true;  
      accessPointPasswordResetBtnPressedTime = 0;
    }


    if(WiFi.status() != WL_CONNECTED)
      wifiInitialized = false;
    else if(!wifiInitialized)
    {
      Serial.print("Access Point IP: ");
      Serial.println(WiFi.localIP());
      wifiInitialized = true;
    }

    //if(WiFi.status() && 
    // TODO: check if sensor states hostory exists and if so save these states to the
    // spereadsheet https://docs.google.com/spreadsheets/d/1TkAlMnVnXBWtPnGmd1scvsLT7g7rjA1LxRSghtyn-48/edit#gid=0 by using its
    // Web App API Call: https://script.google.com/macros/s/AKfycbzM92oEPPacHXdE_Aq_YEhMkFd3q18OkqaEyAQXNZSxR0JYIJUz/exec?
    // where, append the motion sensor states asquery parameters. Use the query variable names to match exactly (case sensitive) the column names.
    // Check the App Script project of the spreadsheet for details.
    
    if(WiFi.status() == WL_CONNECTED)
    {
      int ambientDarkness = 0;
//      Report::eAction actions[REPORT_BUFFER_SIZE];
//      unsigned long timestamps[REPORT_BUFFER_SIZE];
//      int numReportEntries = 0;
      
      portENTER_CRITICAL(&resourceLock);
      ambientDarkness = darknessLevel;
      //numReportEntries = gReport.ExportTriggers(actions, timestamps);
      bool hasActivities = gReport.HasActivities();
      portEXIT_CRITICAL(&resourceLock);

      unsigned long currentTimestampInSeconds = millis() / 1000;
      bool clockRecycled = currentTimestampInSeconds < lastReportTimestampInSeconds;
      bool aggregationPeriodElapsed = (currentTimestampInSeconds - lastReportTimestampInSeconds) >= settings.reportingInterval;
      if(hasActivities && (clockRecycled || aggregationPeriodElapsed))
      {
        WiFiClientSecure client;
        const int httpPort = 443;
        if (!client.connect(LOGGER_URL_HOST, httpPort)) 
        {
          Serial.println("Connection failed");
          return;
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

        String dataEncodedUrl = loggerUrlPath + params;
          //Serial.println(dataEncodedUrl);
          Serial.println(params);
          client.print(String("GET ") + dataEncodedUrl +" HTTP/1.1\r\n" +
             "Host: " + LOGGER_URL_HOST + "\r\n" + 
             "Connection: close\r\n\r\n");

          //Serial.println(dataEncodedUrl);

          client.stop();

          lastReportTimestampInSeconds = currentTimestampInSeconds;
      }

      if(FetchScheduleFlag)
      {
        ReloadSchedule();
        FetchScheduleFlag = false;        
      }
       
   }
   
    delay(500);

  }
}

void ReloadSchedule()
{
  Serial.println("Fetching schedule ...");
  String schedule = fetchSchedule(SCHEDULE_RETREIAVER_HOST, 443, SCHEDULE_RETRIEVER_URL);
  Serial.println(schedule);
  
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
}

String fetchSchedule(const char* host, int port, const char* url)
{
  WiFiClientSecure client;
  if (!client.connect(host, port)) 
  {
    Serial.println("Connection failed");
    return "";
  }
  
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
   
    return fetchSchedule(host2.c_str(), 443, url2.c_str());
  }
  else
  {
    String result = "";
    while (client.available())
    {
      String line = client.readStringUntil('\n');
      line.trim();
      if(result.length() == 0)
        result = line;
      else
        result = result + "," + line;
    }

    client.stop();
    return result;
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
