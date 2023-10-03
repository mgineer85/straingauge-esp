#pragma once

#define CONFIG_DIR "/config/"

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
    virtual void toJson(StaticJsonDocument<512> &doc) const {};
    virtual void fromJson(StaticJsonDocument<512> const &doc){};

    // Loads the configuration from a file
    void loadConfiguration()
    {
        log_d("reading config file");
        log_d("%s", (String(CONFIG_DIR) + this->_filename).c_str());

        // Open file for reading
        File file = FFat.open(String(CONFIG_DIR) + this->_filename, FILE_READ);

        // Allocate a temporary JsonDocument
        // Don't forget to change the capacity to match your requirements.
        // Use arduinojson.org/v6/assistant to compute the capacity.
        StaticJsonDocument<512> doc;

        // Deserialize the JSON document
        DeserializationError error = deserializeJson(doc, file);
        if (error)
            log_e("Deserialization failed, using default configuration");

        this->fromJson(doc);

        // Close the file (Curiously, File's destructor doesn't close the file)
        file.close();

        // save defaults for next time loading.
        // if (error)
        //     this->saveConfiguration();
    }

    // Saves the configuration to a file
    void saveConfiguration()
    {
        log_d("writing config file");
        log_d("%s", (String(CONFIG_DIR) + this->_filename).c_str());

        // Delete existing file, otherwise the configuration is appended to the file
        FFat.remove(String(CONFIG_DIR) + this->_filename);

        // Open file for writing
        File file = FFat.open(String(CONFIG_DIR) + this->_filename, FILE_WRITE, true);
        if (!file)
        {
            log_e("Failed to create file");
            return;
        }
        StaticJsonDocument<512> doc;
        this->toJson(doc);

        // Serialize JSON to file
        if (serializeJson(doc, file) == 0)
            log_e("Failed to write to file");

        // Close the file
        file.close();
    }
};

struct SystemConfig : BaseConfig
{
public:
    using BaseConfig::BaseConfig; // Inherit BaseConfig's constructors.

    // data
public:
    String hostname = "sg-box";

    bool wifi_ap_mode = true; // if true: AP mode, otherwise STA mode
    String wifi_ap_ssid = "sg-box-spot";
    String wifi_ap_password = "12345678";

    String wifi_sta_ssid = "";
    String wifi_sta_password = "";

    // create doc from data
    void toJson(StaticJsonDocument<512> &doc) const
    {
        // Set the values in the document
        doc["hostname"] = hostname;
        doc["wifi_ap_mode"] = wifi_ap_mode;
        doc["wifi_ap_ssid"] = wifi_ap_ssid;
        doc["wifi_ap_password"] = wifi_ap_password;
        doc["wifi_sta_ssid"] = wifi_sta_ssid;
        doc["wifi_sta_password"] = wifi_sta_password;
    };

    // set data according to doc
    void fromJson(StaticJsonDocument<512> const &doc)
    {
        // Copy values from the JsonDocument to the Config
        hostname = doc["hostname"] | hostname;
        wifi_ap_mode = doc["wifi_ap_mode"] | wifi_ap_mode;
        wifi_ap_ssid = doc["wifi_ap_ssid"] | wifi_ap_ssid;
        wifi_ap_password = doc["wifi_ap_password"] | wifi_ap_password;
        wifi_sta_ssid = doc["wifi_sta_ssid"] | wifi_sta_ssid;
        wifi_sta_password = doc["wifi_sta_password"] | wifi_sta_password;
    };
};

struct SensorConfig : BaseConfig
{
    // public:
    //  SensorConfig() : BaseConfig(String("config_sensor.json")){}; // constructor
    using BaseConfig::BaseConfig; // Inherit BaseConfig's constructors.
public:
    // data
    char name[64] = "Default Sensor";
    char serial[64] = "";
    float fullrange = 100;
    float sensitivity = 1.5;
    float zerobalance = 0.0;
    char displayunit[16] = "%";

    // create doc from data
    void toJson(StaticJsonDocument<512> &doc) const
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
    void fromJson(StaticJsonDocument<512> const &doc)
    {
        // Copy values from the JsonDocument to the Config
        strlcpy(name, doc["name"] | name, sizeof(name));
        strlcpy(serial, doc["serial"] | "", sizeof(serial));
        fullrange = doc["fullrange"] | fullrange;
        sensitivity = doc["sensitivity"] | sensitivity;
        zerobalance = doc["zerobalance"] | zerobalance;
        strlcpy(displayunit, doc["displayunit"] | displayunit, sizeof(displayunit));
    };
};