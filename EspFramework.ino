#include <FS.h>
#include <ESP8266WiFi.h>
#include <ESP8266NetBIOS.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>

String ssid = "";
String pass = "";
String name = "";

ESP8266WebServer server(80);

void startFlash();
void getConfig();
void setupWifiClient();
void startServer();
void sendConfig();

void setup(void) 
{
  //Start the serial and clear the garbage
  Serial.begin(115200);
  Serial.println("");
  Serial.println("");

  //Start the SPIFFS
  startFlash();

  //Get the ssid and pass
  getConfig();

  //Connect to the wifi
  setupWifiClient();

  //Start the server
  startServer();
}

void loop() 
{
  server.handleClient();
  delay(10);
}

void startFlash()
{
  Serial.println("Starting SPIFFS");
  if (!SPIFFS.begin())
  {
    Serial.println("SPIFFS mount failed");
  }
  else 
  {
    Serial.println("SPIFFS mount succesful");
  }
}

void getConfig()
{
  Serial.println("Getting configuration data");
  File conf = SPIFFS.open("/conf", "r");
  if (!conf)
  {
    Serial.println("Could not read configuration");
    return;
  }

  ssid = conf.readStringUntil('\n');
  Serial.println("SSID: " + ssid);
  
  pass = conf.readStringUntil('\n');
  Serial.println("Pass: " + pass);

  name = conf.readStringUntil('\n');
  Serial.println("Device Name: " + name);
}

void setupWifiClient()
{
  Serial.println("Connecting to network");
  
  WiFi.mode(WIFI_STA);
  WiFi.hostname(name);
  WiFi.begin(ssid.c_str(), pass.c_str());

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  
  NBNS.begin(name.c_str());
  
  Serial.println("");
  Serial.println("Connected to: " + ssid);
  Serial.println("IP Address: " + WiFi.localIP().toString());
}

void startServer()
{
  Serial.println("Starting HTTP server");
  
  server.serveStatic("/", SPIFFS, "/web/index.html");
  server.serveStatic("/mdl", SPIFFS, "/web/mdl", "max-age=86400");
  server.serveStatic("/ui", SPIFFS, "/web/ui");

  server.on("/data/getConfig", sendConfig);
  
  server.begin();
  Serial.println("HTTP server started");
}

void sendConfig()
{
  String json = "{ \"ssid\": \"" + ssid + "\", \"pass\": \"" + pass + "\", \"name\": \"" + name + "\" }";
  server.send(200, "application/json", json);
}

