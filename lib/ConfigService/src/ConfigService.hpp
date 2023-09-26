#pragma once

#include <Arduino.h>
#include <FFat.h>
#include <ArduinoJson.h>

#define CONFIG_DIR "/config/"

///
namespace ConfigService
{
    struct BaseConfig
    {
    public:
        String _filename;

        BaseConfig(String filename)
        {
            // warning! need to specify valid filename in child class.
            _filename = filename;
        }

    public:
        virtual void setDoc(StaticJsonDocument<512> &doc) const {};
        virtual void setStruct(StaticJsonDocument<512> const &doc){};
    };

    void initialize();
    void update_loop();

    void loadConfiguration(BaseConfig &config);
    void saveConfiguration(const BaseConfig &config);

    void printFile(const String filename);
    void printDirectory(File dir, int numTabs = 2);
}