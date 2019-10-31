
void initConfigServer()
{
  // Setting up access point
  Serial.print("Setting up Access Point …");
  WiFi.softAP(settings.accessPointSsid, settings.accessPointPassword);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);


  // Registering routes for the web server
  server.on("/", HTTP_GET, handleRoot);

  server.on("/wifi", HTTP_GET, [](AsyncWebServerRequest * request){ handleSsidGet(request, "WiFi Settings", "/wifi", settings.wifiSsid);});
  server.on("/wifi", HTTP_POST, [](AsyncWebServerRequest * request){ handleSsidPost(request, settings.wifiSsid, settings.wifiPassword, true, true, false);});

  server.on("/ap", HTTP_GET, [](AsyncWebServerRequest * request){ handleSsidGet(request, "Access Point Settings", "/ap", settings.accessPointSsid);});
  server.on("/ap", HTTP_POST, [](AsyncWebServerRequest * request){ handleSsidPost(request, settings.accessPointSsid, settings.accessPointPassword, true, false, true);});

  server.on("/lighting", HTTP_GET, [](AsyncWebServerRequest * request){ handleLightingSettingsGet(request);});
  server.on("/lighting", HTTP_POST, [](AsyncWebServerRequest * request){ handleLightingSettingsPost(request);});

  server.on("/testmode", HTTP_GET, [](AsyncWebServerRequest * request){ handleTestModeGet(request);});
  server.on("/testmode", HTTP_POST, [](AsyncWebServerRequest * request){ handleTestModePost(request);});

  server.on("/scheduleapi", HTTP_GET, [](AsyncWebServerRequest * request){ handleScheduleApiGet(request);});
  server.on("/scheduleapi", HTTP_POST, [](AsyncWebServerRequest * request){ handleScheduleApiPost(request);});

//  server.on("/schedule", HTTP_GET, [](AsyncWebServerRequest * request){ handleScheduleShow(request);});
//  server.on("/schedule", HTTP_POST, [](AsyncWebServerRequest * request){ handleScheduleReloadReload(request);});
  
  // Starting the webserver
  server.begin();
  
}

void handleRoot(AsyncWebServerRequest * request)
{  
  bool enableChanges = checkAccess(request);
  
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->print(htmlPageHead());
  response->print("<h2>Access Point Hotspot Settings</h2>");
  response->print("<div>");
  response->print("SSID: "); response->print(settings.accessPointSsid);
  response->print("<br />SSID Password: ");
  if(strcmp(settings.accessPointPassword, DEFAULT_AP_PW) == 0)
  {
    response->print(settings.accessPointPassword);
    response->print(" <span class='red'>IMPORTANT: change this default password.</span>");
  }
  else
    response->print("********");

  if(enableChanges)
    response->print("<br /><a href='/ap'>[Update]</a> ");    
  response->print("</div>");

  response->print("<h2>WiFi Settings</h2>");
  response->print("<div>");
  response->print("SSID: "); response->print(settings.wifiSsid);
  response->print("<br />");
  if(strlen(settings.wifiPassword) > 0)
    response->print("WiFi Password: ********");
  else
    response->print("WiFi Password:");

  if(enableChanges)
    response->print("<br /><a href='/wifi'>[Update]</a> ");    
  response->print("</div>");

  response->print("<h2>Lighting Settings</h2>");
  response->print("<div><ul>");
  response->print("<li>Regular Lamps-on Period: "); response->print(settings.regularLampOnTime); response->print(" seconds</li>");
  response->print("<li>Short Lamps-on Period: "); response->print(settings.auxiliaryLampOnTime); response->print(" seconds</li>");
  response->print("<li>Inter-segment Time Delay: "); response->print(settings.interSegmentDelay); response->print(" seconds</li>");
  response->print("<li>Day-light Threshold (0 - 4095): "); response->print(settings.dayLightThreshold); response->print("</li>");
  response->print("</ul>");
  if(enableChanges)    
    response->print("<a href='/lighting'>[Update]</a> ");    
  response->print("</div><br />");

  response->print("<h2>Schedule API Settings</h2>");
  response->print("<div>");
  response->printf("Schedule API URL: https://%s%s<br />", settings.scheduleApiHost, settings.scheduleApiPath);
  if(enableChanges)    
    response->print("<a href='/scheduleapi'>[Update]</a> ");    
  response->print("</div><br />");

  if(testMode)
    response->print("<div class='red'>System is in test mode.</div>");
  else
    response->print("<div class='green'>System is in operational mode.</div>");

  if(enableChanges)
    response->print("<div><a href='/testmode'>[Change Mode]</a></div>");
  response->print(htmlPageTail(false));
  
  request->send(response);  
}


void handleSsidGet(AsyncWebServerRequest * request, String formTitle, String postPath, const char* ssid)
{
  if(!authorizeAccess(request))
    return;
      
  request->send(200, "text/html", 
    htmlPageHead() +
    "<h2>" + formTitle + "</h2>"
    "<form method='POST' action='" + postPath + "'>"
      "<label>SSID:</label> <input type='text' name='ssid', value='" + String(ssid) + "'><br />"
      "<label>Password:</label> <input type='password' name='ssidpw', value=''> (leave blank to retain current password)<br /><br />"
      "<input type='submit' value='Update'>"
    "</form>" + 
    htmlPageTail(true)
  ); 
}

void handleSsidPost(AsyncWebServerRequest * request, char* ssid, char* ssidpw, bool saveSettings, bool restartWifiIfCredentialsChanged, bool restartAccessPointIfCredentialsChanged)
{
  if(!authorizeAccess(request))
    return;
  
  String str;
  bool credentialsChanged = false;
  if(request->hasParam("ssid", true))
  {
    AsyncWebParameter* p = request->getParam("ssid", true);
    str = p->value();
    str.trim();
    
    if(String(ssid) != str)
      credentialsChanged = true;
      
    str.toCharArray(ssid, SSID_MAX_LENGTH-1);
  }
  
  if(request->hasParam("ssidpw", true))
  {
    AsyncWebParameter* p = request->getParam("ssidpw", true);
    str = p->value();
    str.trim();

    if(str.length() > 0)
    {
      if(String(ssidpw) != str)
        credentialsChanged = true;

      str.toCharArray(ssidpw, SSID_PASSWORD_MAX_LENGTH-1);
    }     
  }

  if(credentialsChanged && saveSettings)
  {
    EEPROM.put(0, settings);
    EEPROM.commit();
  }

  showSettings(settings);

  if(credentialsChanged && restartWifiIfCredentialsChanged)
  {
    while(WiFi.status() == WL_CONNECTED)
    {
      WiFi.disconnect(true);
      delay(500);
    }
    
    WiFi.begin(ssid, ssidpw);
    Serial.println("Re-initialized WiFi SSID and Password");
    delay(500);
    
  }

  if(credentialsChanged && restartAccessPointIfCredentialsChanged)
  {
    Serial.print("Resetting up Access Point …");
    WiFi.softAP(settings.accessPointSsid, settings.accessPointPassword);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
  }

  request->send(200, "text/html", 
    htmlPageHead() +
    "<h2>Settings saved!</h2>" + 
    htmlPageTail(true)
  );
}

void handleLightingSettingsGet(AsyncWebServerRequest * request)
{
  if(!authorizeAccess(request))
    return;
      
  request->send(200, "text/html", 
    htmlPageHead() +
    "<h2>Lighting System Parameters</h2>"
    "<form method='POST' action='/lighting'>"
      "<label>Regular Lamp-on Time:</label> <input type='number' name='regOnTime', value='" + String(settings.regularLampOnTime) + "'><br />"
      "<label>Short Lamp-on Time:</label> <input type='number' name='auxOnTime', value='" + String(settings.auxiliaryLampOnTime) + "'><br />"
      "<label>Inter-segment Time Delay:</label> <input type='number' name='intSegDelay', value='" + String(settings.interSegmentDelay) + "'><br />"
      "<label>Day-light Threshold (0 - 4095):</label> <input type='number' name='daylightThreshold', value='" + String(settings.dayLightThreshold) + "'><br />"
      "<input type='submit' value='Update'>"
    "</form>" + 
    htmlPageTail(true)
  ); 
}

void handleLightingSettingsPost(AsyncWebServerRequest * request)
{
  if(!authorizeAccess(request))
    return;
    
  bool settingsChanged = false;
  int val;

  val = getIntParam(request, "regOnTime", -1);
  if(val > 0)
  {
    if(settings.regularLampOnTime != val)
    {
      settings.regularLampOnTime = val;
      settingsChanged = true;
    }
  }

  val = getIntParam(request, "auxOnTime", -1);
  if(val > 0)
  {
    if(settings.auxiliaryLampOnTime != val)
    {
      settings.auxiliaryLampOnTime = val;
      settingsChanged = true;
    }
  }
  
  val = getIntParam(request, "intSegDelay", -1);
  if(val > 0)
  {
    if(settings.interSegmentDelay != val)
    {
      settings.interSegmentDelay = val;
      settingsChanged = true;
    }
  }

  val = getIntParam(request, "daylightThreshold", -1);
  if(val > 0)
  {
    if(settings.dayLightThreshold != val)
    {
      settings.dayLightThreshold = val;
      settingsChanged = true;
    }
  }

  if(settingsChanged)
  {
    EEPROM.put(0, settings);
    EEPROM.commit();
    delay(500);
  }

  request->send(200, "text/html", 
    htmlPageHead() +
    "<h2>Settings saved!</h2>" + 
    htmlPageTail(true)
  );  
}

void handleTestModeGet(AsyncWebServerRequest * request)
{
  if(!authorizeAccess(request))
    return;

  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->print(htmlPageHead());
  response->print("<h2>Test Mode</h2>");
  
  sendTestModeSettings(response);
  
  response->print("<br /><form method='POST' action='/testmode'>");
  if(testMode)
    response->print("<input type='submit' name='submit' value='Disable Test Mode'>");
  else
    response->print("<input type='submit' name='submit' value='Enable Test Mode'>");    

  response->print("</form>");

  response->print(htmlPageTail(!testMode));

  request->send(response);  
}

void handleTestModePost(AsyncWebServerRequest * request)
{
  if(!authorizeAccess(request))
    return;

  if(request->hasParam("submit", true))
  {
    Serial.println("submit true!!");

    AsyncWebParameter* p = request->getParam("submit", true);
    String str = p->value();

    if(str == "Enable Test Mode")
    {
      // Turn on test mode and adjust lighting parameters to test-mode vales
      Serial.println("Entering test mode!!");
      testMode = true;
      settings.regularLampOnTime = TEST_REG_LAMP_ON_TIME;
      settings.auxiliaryLampOnTime = TEST_AUX_LAMP_ON_TIME;
      settings.interSegmentDelay = TEST_INTER_SEG_DELAY;
      settings.dayLightThreshold = TEST_DAYLIGHT_THRESHOLD;     
    }
    else
    {
      // Turn off test mode and reload settings from the EEPROM
      Serial.println("Exiting test mode!!");   
      testMode = false;   
      loadConfigSettings(settings);       
    }

     // resetting lamp-state timers
    portENTER_CRITICAL(&resourceLock);
    gSegmentA.ResetAll();
    gSegmentB.ResetAll();
    gSegmentC.ResetAll();
    gSegmentD.ResetAll();
    gSegmentE.ResetAll();
    portEXIT_CRITICAL(&resourceLock);   

    handleTestModeGet(request);
  }
} 


void handleScheduleApiGet(AsyncWebServerRequest * request)
{
  if(!authorizeAccess(request))
    return;
      
  request->send(200, "text/html", 
    htmlPageHead() +
    "<h2>Update Schedule API URL</h2>"
    "<b>Schedule API must use standard HTTPS protocol on port 443.</b>"
    "<form method='POST' action='/scheduleapi'>"
      "<label>API URL Host:</label> <input type='text' name='scheduleApiHost', value='" + String(settings.scheduleApiHost) + "'><br />"
      "<label>API URL URL Path:</label> <input type='text' name='scheduleApiPath', value='" + String(settings.scheduleApiPath) + "'><br />"
      "<input type='submit' value='Update'>"
    "</form>" + 
    htmlPageTail(true)
  ); 
}

void handleScheduleApiPost(AsyncWebServerRequest * request)
{
  if(!authorizeAccess(request))
    return;
  
  String host, path;
  if(request->hasParam("scheduleApiHost", true) && request->hasParam("scheduleApiPath", true))
  {
    AsyncWebParameter* p = request->getParam("scheduleApiHost", true);
    host = p->value();
    host.trim();

    p = request->getParam("scheduleApiPath", true);
    path = p->value();
    path.trim();
    if(!path.startsWith("/"))
    {
      path = "/" + path;
    }

    if(host != settings.scheduleApiHost || path != settings.scheduleApiPath)
    {
      host.toCharArray(settings.scheduleApiHost, HOST_NAME_MAX_LENGTH-1);
      path.toCharArray(settings.scheduleApiPath, URL_MAX_LENGTH-1);

      EEPROM.put(0, settings);
      EEPROM.commit();
      showSettings(settings);
    }

    request->send(200, "text/html", 
     htmlPageHead() +
     "<h2>Settings saved!</h2>" + 
     htmlPageTail(true)
    );
  }
  else
  {
    request->send(200, "text/html", 
     htmlPageHead() +
     "<h2>Settings not saved!</h2>" + 
     "<p>Parameter(s) missing</p>" + 
     htmlPageTail(true)
    );
  }
}
/*
void handleScheduleShow(AsyncWebServerRequest * request)
{
  if(!authorizeAccess(request))
    return;

  bool enableChanges = checkAccess(request);

  //copying start times of active or pending schedules
  
  // Using bubble sort to sort 
  
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->print(htmlPageHead());
  response->print("<h2>Lighting Schedule</h2>");



  if(enableChanges)
    response->print("<div><form method='POST' action='/schedule'><input type='submit' value='Reload Schedule'></form></div>");
  response->print(htmlPageTail(false));
  
  request->send(response);   
}

void handleScheduleReloadReload(AsyncWebServerRequest * request)
{
  
}
*/
void sendTestModeSettings(AsyncResponseStream *response)
{
  response->print("The test mode uses following parameters: <br />");
  response->print("<div>");
  response->print("Regular Lamps-on Period: "); response->print(TEST_REG_LAMP_ON_TIME); response->print(" seconds"); response->print("<br />");
  response->print("Short Lamps-on Period: "); response->print(TEST_AUX_LAMP_ON_TIME); response->print(" seconds"); response->print("<br />");
  response->print("Inter-segment Time Delay: "); response->print(TEST_INTER_SEG_DELAY); response->print(" seconds"); response->print("<br />");
  response->print("Day-light Threshold (0 - 4095): "); response->print(TEST_DAYLIGHT_THRESHOLD); response->print("<br />");    
  response->print("</div>"); 
}

int getIntParam(AsyncWebServerRequest * request, const char* paramName, int defaultVal)
{
  String str;
  char buff[8];
  if(request->hasParam(paramName, true))
  {
    AsyncWebParameter* p = request->getParam(paramName, true);
    str = p->value();
    str.toCharArray(buff, 8);
    unsigned int val = atoi(buff);
    return val;
  }

  return defaultVal;  
}

bool authorizeAccess(AsyncWebServerRequest* request)
{
  if(checkAccess(request))
  {
    return true;
  }
  else
  {
    request->send(200, "text/html", 
      htmlPageHead() +
      "<h2>Authorization Faild</h2>"
      "<p>Please connect through the access point hotspot <b>" + String(settings.accessPointSsid) +"</b> to access this page.</p>" + 
      htmlPageTail(true)
    );
    return false;
  }
}

bool checkAccess(AsyncWebServerRequest* request)
{
  // Authorize only individuals who connect through the Access Point hotspot unless the program code specifically 
  // sets ENABLE_CONFIGRATION_OVER_LAN flag true (which is only recommended to do in development mode).

  IPAddress clientIp = request->client()->remoteIP();
  IPAddress apIp = WiFi.softAPIP();
  //Serial.print("Client IP: ");
  //Serial.println(clientIp);
  //Serial.print("AP IP: ");
  //Serial.println(apIp);
    
  return ENABLE_CONFIGRATION_OVER_LAN || (clientIp[0] == apIp[0] && clientIp[1] == apIp[1] && clientIp[2] == apIp[2]);
}

String htmlPageHead()
{
  return
    "<!DOCTYPE html>"
    "<html>"
      "<head>"
        "<style>"
          "body{font-family: arial,sans-serif; margin-bottom: 20px;}"
          "input{font-size: 1em;}"
          "@media (max-width: 992px){ body{font-size: 1.75em;}}"
          ".red{color:red;}"
          ".green{color:#006400;}"
          "div{margin-left: 10px; margin-right: 10px}"
        "</style>"
      "</head>"
      "<body>"
        "<h1>Lighting Controller</h1>"
  ;
}

String htmlPageTail(bool includeHomeLink)
{
  if(includeHomeLink)
    return
          "<br /></br/><a href='/'>[Home]</a>"
        "</body>"
      "</html>"
    ;
  else
    return
        "</body>"
      "</html>"
    ;
}
