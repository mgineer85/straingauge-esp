#pragma once

#include <Arduino.h>
#include <FFat.h>
#include <ArduinoJson.h>
#include "ConfigStructs.hpp"

#define CONFIG_DIR "/config/"

class ConfigClass
{
public:
    SystemConfig system_config = SystemConfig("config_system.json");
    SensorConfig sensor_config = SensorConfig("config_sensor.json");

    ConfigClass();

    void loadConfiguration(BaseConfig &config);
    void saveConfiguration(const BaseConfig &config);
};

extern ConfigClass g_Config;
