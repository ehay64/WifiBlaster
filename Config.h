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

        String getSsid();
        String getPass();
        String getName();
};

#endif
