#pragma once

#include <Arduino.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <ConfigStructs.hpp>
#include <FFat.h>
#include <DataEvent.hpp>
#include <WiFi.h>
#include <AsyncTCP.h>

/// The display module to control the attached LEDs
///
namespace Webservice
{

    /// Initialize the webserver
    void initialize();

    void invokeSendEvent(String event, String value);

}