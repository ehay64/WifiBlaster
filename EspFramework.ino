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
int setupWifiClient();
void runPortal();
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
  if (setupWifiClient() != 0)
  {
    //If we can't connect to a network, start a captive portal
    runPortal();
  }

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

int setupWifiClient()
{
  Serial.println("Connecting to network: " + ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.hostname(name);
  WiFi.begin(ssid.c_str(), pass.c_str());

  for (int i = 0; i < 60; i++)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      break;
    }
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Couldn't connect to network: " + ssid);
    WiFi.disconnect();
    return 1;
  }

  NBNS.begin(name.c_str());
  Serial.println("Connected to: " + ssid);
  Serial.println("IP Address: " + WiFi.localIP().toString());
  return 0;
}

void runPortal()
{
  Serial.println("Starting Captive Portal Setup");

  Serial.println("Setting up AP: ESP Framework");
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP Framework", "password");

  Serial.println("Starting DNS Server");
  IPAddress address(192, 168, 4, 1);
  DNSServer dnsServer;
  dnsServer.start(53, "*", address);

  startServer();

  while (1)
  {
    dnsServer.processNextRequest();
    server.handleClient();
  }
}

void startServer()
{
  Serial.println("Starting HTTP server");

  server.onNotFound([]()
  {
    server.sendHeader("Location", "/", true);
    server.send ( 302, "text/plain", "");
  });
  
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

