#ifndef Config_h
#define Config_h

#include <Arduino.h>
#include <FS.h>

class Config
{
    private:
        String ssid;
        String pass;
        String name;

    public:
        Config();

        void loadConfig();
        void saveConfig();

        String getSsid();
        String getPass();
        String getName();

        void setSsid(String s);
        void setPass(String p);
        void setName(String n);
};

#endif
