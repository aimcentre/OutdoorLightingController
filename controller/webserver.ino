
void initServer()
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

  server.on("/wifi", HTTP_POST, [](AsyncWebServerRequest * request){ handleSsidPost(request, settings.wifiSsid, settings.wifiPassword, true);});
  server.on("/ap", HTTP_POST, [](AsyncWebServerRequest * request){ handleSsidPost(request, settings.accessPointSsid, settings.accessPointPassword, true);});

  // Starting the webserver
  server.begin();
  
}

void handleRoot(AsyncWebServerRequest * request)
{  
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->print("<!DOCTYPE html><html><head><title>Lighting Controller Settings</title>"
    "<style>"
      "div{margin-left: 10px; margin-right: 10px}"
    "</style>"
    "</head><body>");
  response->print("<h1>Lighting Controller Settings</h1>");
  response->print("<h2>Access Point Settings</h2>");
  response->print("<div>");
  response->print("SSID: "); response->print(settings.accessPointSsid);
  response->print("<br />SSID Password: ");
  if(strcmp(settings.accessPointPassword, DEFAULT_AP_PW) == 0)
    response->print(settings.accessPointPassword);
  else
    response->print("********");
  response->print("<br /><a href='/ap'>update</a> ");    
  response->print("</div>");

  response->print("<h2>WiFi Settings</h2>");
  response->print("<div>");
  response->print("SSID: "); response->print(settings.wifiSsid);
  response->print("<br />");
  if(strlen(settings.wifiPassword) > 0)
    response->print("WiFi Password: ********");
  else
    response->print("WiFi Password:");
   
  response->print("<br /><a href='/wifi'>update</a> ");    
  response->print("</div>");
   
  response->print("</body></html>");
  
  request->send(response);  
}


void handleSsidGet(AsyncWebServerRequest * request, String formTitle, String postPath, const char* ssid)
{
  request->send(200, "text/html", 
    "<html>"
      "<head>"
      "</head>"
      "<body>"
        "<h1>" + formTitle + "</h1>"
        "<form method='POST' action='" + postPath + "'>"
          "<label>SSID:</label> <input type='text' name='ssid', value='" + String(ssid) + "'><br />"
          "<label>Password:</label> <input type='password' name='ssidpw', value=''> (leave blank to keep current password unchanged)<br />"
          "<input type='submit' value='Update'>"
        "</form>"
      "</body>"
    "</html>"
  ); 
}

void handleSsidPost(AsyncWebServerRequest * request, char* ssid, char* ssidpw, bool saveSettings)
{
  String str;
  if(request->hasParam("ssid", true))
  {
    AsyncWebParameter* p = request->getParam("ssid", true);
    str = p->value();
    str.trim();
    str.toCharArray(ssid, SSID_MAX_LENGTH-1);
  }
  
  if(request->hasParam("ssidpw", true))
  {
    AsyncWebParameter* p = request->getParam("ssidpw", true);
    str = p->value();
    str.trim();
    if(str.length() > 0)
    {
      str.toCharArray(ssidpw, SSID_PASSWORD_MAX_LENGTH-1);
    }
  }

  if(saveSettings)
  {
    EEPROM.put(0, settings);
    EEPROM.commit();
  }

  showSettings(settings);

  request->send(200, "text/html", "<html><body><h2>Settings saved!</h2><br /><a href='/'>Home</a></body>");
}
