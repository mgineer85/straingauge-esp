#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <FFat.h>
#include <ConfigStructs.hpp>
#include <Display.hpp>

class SystemClass
{
private:
    // WiFi config;

public:
    SystemConfig system_config = SystemConfig("system.json");

    SystemClass();

    void initialize();
    void initialize_i2c();
    void initialize_filesystem();
    void initialize_wifi();

    void cbSaveConfiguration(void);
    void cbLoadConfiguration(void);
    void postConfigChange(void);

    void update_loop();

    void printFilesystemFiles();
    void printFile(const String filename);
    void printDirectory(File dir, int numTabs = 1);
};

extern SystemClass g_System;
