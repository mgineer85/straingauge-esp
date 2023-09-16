#pragma once

#include <Arduino.h>

///
namespace ConfigService
{
    struct Config
    {
        char hostname[64];
        int port;
    };

    void initialize();
    void update_loop();

    void loadConfiguration(const char *filename, Config &config);
    void saveConfiguration(const char *filename, const Config &config);
    void printFile(const char *filename);
}