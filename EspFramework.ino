#include <ESP8266WiFi.h>
#include <ESP8266NetBIOS.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>

#include "Config.h"

Config conf;

ESP8266WebServer server(80);

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

  //Load the configuration data
  conf.loadConfig();

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
}

int setupWifiClient()
{
  Serial.println("Connecting to network: " + conf.getSsid());
  
  WiFi.mode(WIFI_STA);
  WiFi.hostname(conf.getName());
  WiFi.begin(conf.getSsid().c_str(), conf.getPass().c_str());

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
    Serial.println("Couldn't connect to network: " + conf.getSsid());
    WiFi.disconnect();
    return 1;
  }

  NBNS.begin(conf.getName().c_str());
  Serial.println("Connected to: " + conf.getSsid());
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

  server.on("/data/getConfig", getConfig);
  
  server.begin();
  Serial.println("HTTP server started");
}

void getConfig()
{
  String json = "{ \"ssid\": \"" + conf.getSsid() + "\", \"pass\": \"" + conf.getPass() + "\", \"name\": \"" + conf.getName() + "\" }";
  server.send(200, "application/json", json);
}

