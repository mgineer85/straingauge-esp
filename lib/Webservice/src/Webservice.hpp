#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <ConfigService.hpp>
#include <FFat.h>
#include <DataEvent.hpp>
#include "../../../src/utils/filesystem.hpp"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

/// The display module to control the attached LEDs
///
namespace Webservice
{

    /// Initialize the webserver
    void initialize();

    void invokeSendEvent(String event, String value);

}