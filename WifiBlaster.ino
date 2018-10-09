#include <ESP8266WiFi.h>
#include <ESP8266NetBIOS.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>

#include <IRremoteESP8266.h>
#include <IRsend.h>

#include "Config.h"

Config conf;

ESP8266WebServer server(80);

IRsend irsend(4);

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

  //Start the IR transmitter
  irsend.begin();

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
    server.send (302, "text/plain", "");
  });

  server.on("/favicon.ico", []()
  {
    server.send(404, "text/plain", "404: Not found");
  });
  
  server.serveStatic("/", SPIFFS, "/web/index.html");
  server.serveStatic("/mdl", SPIFFS, "/web/mdl", "max-age=86400");
  server.serveStatic("/ui", SPIFFS, "/web/ui");

  server.on("/data/getConfig", getConfig);
  server.on("/data/setConfig", setConfig);

  server.on("/data/sendCode", sendCode);
  
  server.begin();
  Serial.println("HTTP server started");
}

void getConfig()
{
  String json = "{ \"ssid\": \"" + conf.getSsid() + "\", \"pass\": \"" + conf.getPass() + "\", \"name\": \"" + conf.getName() + "\" }";
  server.send(200, "application/json", json);
}

void setConfig()
{
  Serial.println("Setting device configuration");

  if (server.arg("ssid") == "")
  {
    Serial.println("SSID not given");
  }
  else
  {
    Serial.println("Setting new SSID: " + server.arg("ssid"));
    conf.setSsid(server.arg("ssid"));
  }

  if (server.arg("pass") == "")
  {
    Serial.println("Password not given");
  }
  else
  {
    Serial.println("Setting new password: " + server.arg("pass"));
    conf.setPass(server.arg("pass"));
  }

  if (server.arg("name") == "")
  {
    Serial.println("Name not given");
  }
  else
  {
    Serial.println("Setting new name: " + server.arg("name"));
    conf.setName(server.arg("name"));
  }

  server.send(200, "text/plain");
  conf.saveConfig();
  Serial.println("New configuration saved");
}

void sendCode()
{
  Serial.println("Sending IR code");

  uint64_t code = 0;

  if (server.arg("moto") != "")
  {
    code = strtoull(server.arg("moto").c_str(), NULL, 0);

    Serial.print("Sending Motorola: ");
    Serial.print(uint32_t(code >> 32), HEX);
    Serial.println(uint32_t(code), HEX);

    irsend.sendGeneric(9000, 4500, 500, 2200, 500, 4500, 500, 30000, code, 16, 38, true, 0, 50);
  }

  if (server.arg("pana") != "")
  {
    code = strtoull(server.arg("pana").c_str(), NULL, 0);

    Serial.print("Sending Panasonic: ");
    Serial.print(uint32_t(code >> 32), HEX);
    Serial.println(uint32_t(code), HEX);

    irsend.sendPanasonic64(code, 48, 2);
  }

  server.send(200, "text/plain");
}
