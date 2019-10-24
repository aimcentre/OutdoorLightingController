#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

char *WifiSSID = NULL, *WifiPassword = NULL;

volatile bool FetchScheduleFlag = false;

void systemAdminProcess(void * parameter) {

  // Initializing the lamp-schedule timer
  lampScheduleTimer = timerBegin(1, 80, true);
  timerAttachInterrupt(lampScheduleTimer, &fetchScheduleISR, true);
  timerAlarmWrite(lampScheduleTimer, SCHEDULE_CHECK_INTERVAL_SEC * 1000000, true);
  timerAlarmEnable(lampScheduleTimer);


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
      Report::eAction actions[REPORT_BUFFER_SIZE];
      unsigned long timestamps[REPORT_BUFFER_SIZE];
      int numReportEntries = 0;
      
      portENTER_CRITICAL(&resourceLock);
      ambientDarkness = darknessLevel;
      numReportEntries = gReport.ExportTriggers(actions, timestamps);
      portEXIT_CRITICAL(&resourceLock);

      if(numReportEntries > 0)
      {
          WiFiClientSecure client;
          const int httpPort = 443;
          if (!client.connect(spreadsheetHost, httpPort)) 
          {
            Serial.println("Connection failed");
            return;
          }

          float temperature = getTemperature();

          //Serial.printf("Processing %d report entries \r\n", numReportEntries);
          
          String params = String("?t=") + temperature + "&d=" + ambientDarkness;
                            
          for(int i=0; i<numReportEntries; ++i)
          {
            switch(actions[i])
            {
              case Report::eAction::MSA_TRIGGER: params = params + "&mA=" + timestamps[i]; break;
              case Report::eAction::MSB_TRIGGER: params = params + "&mB=" + timestamps[i]; break;
              case Report::eAction::MSC_TRIGGER: params = params + "&mC=" + timestamps[i]; break;
              case Report::eAction::MSD_TRIGGER: params = params + "&mD=" + timestamps[i]; break;
              case Report::eAction::MSE_TRIGGER: params = params + "&mE=" + timestamps[i]; break;
              case Report::eAction::MSF_TRIGGER: params = params + "&mF=" + timestamps[i]; break;
              case Report::eAction::MSG_TRIGGER: params = params + "&mG=" + timestamps[i]; break;

              case Report::eAction::L1_ON:  params = params + "&s1=1&s1clk=" + timestamps[i]; break;
              case Report::eAction::L2_ON:  params = params + "&s2=1&s2clk=" + timestamps[i]; break;
              case Report::eAction::L3_ON:  params = params + "&s3=1&s3clk=" + timestamps[i]; break;
              case Report::eAction::L4_ON:  params = params + "&s4=1&s4clk=" + timestamps[i]; break;
              case Report::eAction::L5_ON:  params = params + "&s5=1&s5clk=" + timestamps[i]; break;
              case Report::eAction::L1_OFF: params = params + "&s1=0&s1clk=" + timestamps[i]; break;
              case Report::eAction::L2_OFF: params = params + "&s2=0&s2clk=" + timestamps[i]; break;
              case Report::eAction::L3_OFF: params = params + "&s3=0&s3clk=" + timestamps[i]; break;
              case Report::eAction::L4_OFF: params = params + "&s4=0&s4clk=" + timestamps[i]; break;
              case Report::eAction::L5_OFF: params = params + "&s5=0&s5clk=" + timestamps[i]; break;
                
            }
          }

          String dataEncodedUrl = url + params;
          //Serial.println(dataEncodedUrl);
          Serial.println(params);
          client.print(String("GET ") + dataEncodedUrl +" HTTP/1.1\r\n" +
             "Host: " + spreadsheetHost + "\r\n" + 
             "Connection: close\r\n\r\n");

          client.stop();
       }


        if(FetchScheduleFlag)
        {
          String currentTime = fetchTime();
          Serial.println(currentTime);

          /*
        
          WiFiClientSecure client;
          const int httpPort = 443;
          const char* host = "www.googleapis.com";
          if (!client.connect(host, httpPort)) 
          {
            Serial.println("Connection failed");
            return;
          }
      
          Serial.println("Fetching current time ...");
        
          String startTime = "2019-10-23T08:00:00-06:00";
        
          String calendar_url = String(CALENDAR_API_ROOT) + CALENDAR_ID + 
              "/events?orderBy=startTime&singleEvents=true" + 
              "&timeMin=" + startTime + 
              "&key=" + CALENDAR_API_KEY;
      
           Serial.println(calendar_url);

           client.print(String("GET ") + calendar_url +" HTTP/1.1\r\n" +
             "Host: " + host + "\r\n" + 
             "Connection: close\r\n\r\n");

            while (client.connected()) {
              String line = client.readStringUntil('\n');
              if (line == "\r") {
                Serial.println("headers received");
                break;
              }
            }

            while (client.available()) {
              String str = client.readString();
              Serial.println(str);
            }
*/

           FetchScheduleFlag = false;
      /*
          HTTPClient http;
          http.begin(TIME_SERVER_API);
          int httpCode = http.GET();  
          if (httpCode > 0) 
          {
              String payload = http.getString();
              Serial.println(httpCode);
              Serial.println(payload);
            }
          else 
          {
            Serial.println("Error on HTTP request");
          }
      */
              //https://www.googleapis.com/calendar/v3/calendars/abva.org_8hkdqiv3l60hb844mek17rr7rk@group.calendar.google.com/events?key=AIzaSyCTisDVkthQZRXOcQH1mu17gOscxM0R-Y4&timeMin=2019-10-22T06:00:00-06:00
          
        }
       
   }
   
    delay(500);

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

    Serial.println("Fetching current time ...");
  
    client.print(String("GET ") + TIME_SERVER_API +" HTTP/1.1\r\n" +
       "Host: " + TIME_SERVER_HOST + "\r\n" + 
       "Connection: close\r\n\r\n");

    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        Serial.println("headers received");
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
