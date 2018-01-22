#include "Config.h"

Config::Config()
{
    
}

void Config::loadConfig()
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

    Serial.println("Getting configuration data");
    File conf = SPIFFS.open("/conf", "r");
    if (!conf)
    {
        Serial.println("Could not read configuration");
        ssid = "WiFi";
        pass = "password";
        name = "ESPDevice";
        return;
    }

    ssid = conf.readStringUntil('\n');
    Serial.println("SSID: " + ssid);
    
    pass = conf.readStringUntil('\n');
    Serial.println("Pass: " + pass);

    name = conf.readStringUntil('\n');
    Serial.println("Device Name: " + name);
}

String Config::getSsid()
{
    return ssid;
}

String Config::getPass()
{
    return pass;
}

String Config::getName()
{
    return name;
}
