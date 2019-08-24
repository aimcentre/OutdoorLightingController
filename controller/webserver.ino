
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
  server.on("/ap", HTTP_GET, [](AsyncWebServerRequest * request){ handleSsidGet(request, "Access Point Settings", "/ap", settings.accessPointSsid);});

  server.on("/wifi", HTTP_POST, [](AsyncWebServerRequest * request){ handleSsidPost(request, settings.wifiSsid, settings.wifiPassword, true, true);});
  server.on("/ap", HTTP_POST, [](AsyncWebServerRequest * request){ handleSsidPost(request, settings.accessPointSsid, settings.accessPointPassword, true, false);});

  // Starting the webserver
  server.begin();
  
}

void handleRoot(AsyncWebServerRequest * request)
{  
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->print(htmlPageHead());
  response->print("<h2>Access Point Hotspot Settings</h2>");
  response->print("<div>");
  response->print("SSID: "); response->print(settings.accessPointSsid);
  response->print("<br />SSID Password: ");
  if(strcmp(settings.accessPointPassword, DEFAULT_AP_PW) == 0)
  {
    response->print(settings.accessPointPassword);
    response->print(" <span class='red'>IMPORTANT: change this system-default password.</span>");
  }
  else
    response->print("********");
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
   
  response->print("<br /><a href='/wifi'>[Update]</a> ");    
  response->print("</div>");

  response->print("<h2>Lamp Timing</h2>");

  response->print(htmlPageTail(false));
  
  request->send(response);  
}


void handleSsidGet(AsyncWebServerRequest * request, String formTitle, String postPath, const char* ssid)
{
  if(!verifyAccessPointClient(request))
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

void handleSsidPost(AsyncWebServerRequest * request, char* ssid, char* ssidpw, bool saveSettings, bool restartWifiIfCredentialsChanged)
{
  if(!verifyAccessPointClient(request))
    return;
  
  String str;
  bool restartWifi = false;
  if(request->hasParam("ssid", true))
  {
    AsyncWebParameter* p = request->getParam("ssid", true);
    str = p->value();
    str.trim();
    
    if(restartWifiIfCredentialsChanged && String(ssid) != str)
      restartWifi = true;
      
    str.toCharArray(ssid, SSID_MAX_LENGTH-1);
  }
  
  if(request->hasParam("ssidpw", true))
  {
    AsyncWebParameter* p = request->getParam("ssidpw", true);
    str = p->value();
    str.trim();

    if(str.length() > 0)
    {
      if(restartWifiIfCredentialsChanged && String(ssidpw) != str)
        restartWifi = true;

      str.toCharArray(ssidpw, SSID_PASSWORD_MAX_LENGTH-1);
    }     
  }

  if(saveSettings)
  {
    EEPROM.put(0, settings);
    EEPROM.commit();
  }

  showSettings(settings);

  if(restartWifi)
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

  request->send(200, "text/html", 
    htmlPageHead() +
    "<h2>Settings saved!</h2>" + 
    htmlPageTail(true)
  );
}

bool verifyAccessPointClient(AsyncWebServerRequest* request)
{
  IPAddress clientIp = request->client()->remoteIP();
  IPAddress apIp = WiFi.softAPIP();
  Serial.print("Client IP: ");
  Serial.println(clientIp);
  Serial.print("AP IP: ");
  Serial.println(apIp);

  if(clientIp[0] == apIp[0] && clientIp[1] == apIp[1] && clientIp[2] == apIp[2])
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

String htmlPageHead()
{
  return
    "<!DOCTYPE html>"
    "<html>"
      "<head>"
        "<style>"
          ".red{color:red;}"
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
