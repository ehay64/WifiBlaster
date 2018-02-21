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

    conf.close();
}

void Config::saveConfig()
{
    Serial.println("Saving configuration data");
    File conf = SPIFFS.open("/conf", "w");
    if (!conf)
    {
        Serial.println("Could not open configuration");
        return;
    }

    conf.print(ssid + "\n");
    Serial.println("Saved SSID: " + ssid);

    conf.print(pass + "\n");
    Serial.println("Saved Pass: " + pass);

    conf.print(name + "\n");
    Serial.println("Saved Name: " + name);

    conf.print("\n");
    conf.close();
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

void Config::setSsid(String s)
{
    ssid = s;
}

void Config::setPass(String p)
{
    pass = p;
}

void Config::setName(String n)
{
    name = n;
}
