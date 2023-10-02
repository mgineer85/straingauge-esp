#pragma once

#include <Arduino.h>
#include <FFat.h>
#include <ArduinoJson.h>

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

struct SystemConfig : BaseConfig
{
public:
    using BaseConfig::BaseConfig; // Inherit BaseConfig's constructors.

    // data
public:
    String hostname = "sg-box";

    String wifi_ssid = "chumbawumba";
    String wifi_password = "Schneepflug";

    // create doc from data
    void setDoc(StaticJsonDocument<512> &doc) const
    {
        // Set the values in the document
        doc["hostname"] = hostname;
        doc["wifi_ssid"] = wifi_ssid;
        doc["wifi_password"] = wifi_password;
    };

    // set data according to doc
    void setStruct(StaticJsonDocument<512> const &doc)
    {
        // Copy values from the JsonDocument to the Config
        hostname = doc["hostname"] | "sg-box";
        wifi_ssid = doc["wifi_ssid"] | "chumbawumba";
        wifi_password = doc["wifi_password"] | "Schneepflug";
    };
};

struct SensorConfig : BaseConfig
{
    // public:
    //  SensorConfig() : BaseConfig(String("config_sensor.json")){}; // constructor
    using BaseConfig::BaseConfig; // Inherit BaseConfig's constructors.
public:
    // data
    char name[64];
    char serial[64];
    float fullrange;
    float sensitivity;
    float zerobalance;
    char displayunit[16];

    // create doc from data
    void setDoc(StaticJsonDocument<512> &doc) const
    {
        // Set the values in the document
        doc["name"] = name;
        doc["serial"] = serial;
        doc["fullrange"] = fullrange;
        doc["sensitivity"] = sensitivity;
        doc["zerobalance"] = zerobalance;
        doc["displayunit"] = displayunit;
    };

    // set data according to doc
    void setStruct(StaticJsonDocument<512> const &doc)
    {
        // Copy values from the JsonDocument to the Config
        strlcpy(name, doc["name"] | "Default Sensor", sizeof(name));
        strlcpy(serial, doc["serial"] | "", sizeof(serial));
        fullrange = doc["fullrange"] | 100;
        sensitivity = doc["sensitivity"] | 1.5;
        zerobalance = doc["zerobalance"] | 0.0;
        strlcpy(displayunit, doc["displayunit"] | "%", sizeof(displayunit));
    };
};