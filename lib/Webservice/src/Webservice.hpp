#pragma once

#include <Arduino.h>

#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

/// The display module to control the attached LEDs
///
namespace Webservice
{

    /// Initialize the webserver
    ///
    void initialize();

    void invokeSendEvent(String event, String value);

    /// The Actions.
    ///
    enum Actions : uint8_t
    {
        Tare,
        LoadPersistPrefs, // TODO
        SavePersistPrefs, // TODO
        CalibrateFactor,
    };
    /// The callback function.
    ///
    typedef void (*Function)();
    /// Set a callback if the given action is requested
    ///
    void setCallback(Actions action, Function fn);
}