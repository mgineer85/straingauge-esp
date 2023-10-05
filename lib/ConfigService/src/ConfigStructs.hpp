#pragma once

#define CONFIG_DIR "/config/"
#define MAX_DOCUMENT_SIZE 1024

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
    virtual void toDoc(DynamicJsonDocument &doc) const {};
    virtual void fromDoc(DynamicJsonDocument const &doc){};
    virtual void fromWeb(JsonVariant variant){};

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
        DynamicJsonDocument doc(MAX_DOCUMENT_SIZE);

        // Deserialize the JSON document
        DeserializationError error = deserializeJson(doc, file);
        if (error)
            log_e("Deserialization failed, using default configuration");

        this->fromDoc(doc);

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
        DynamicJsonDocument doc(MAX_DOCUMENT_SIZE);
        this->toDoc(doc);

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
    void toDoc(DynamicJsonDocument &doc) const
    {
        // Set the values in the document
        Serial.println(doc["hostname"].as<String>());
        doc["hostname"] = hostname;
        doc["wifi_ap_mode"] = wifi_ap_mode;
        doc["wifi_ap_ssid"] = wifi_ap_ssid;
        doc["wifi_ap_password"] = wifi_ap_password;
        doc["wifi_sta_ssid"] = wifi_sta_ssid;
        doc["wifi_sta_password"] = wifi_sta_password;
    };

    // set data according to doc
    void fromDoc(DynamicJsonDocument const &doc)
    {
        // Copy values from the JsonDocument to the Config
        Serial.println(doc["hostname"].as<String>());
        hostname = doc["hostname"] | hostname;
        wifi_ap_mode = doc["wifi_ap_mode"] | wifi_ap_mode;
        wifi_ap_ssid = doc["wifi_ap_ssid"] | wifi_ap_ssid;
        wifi_ap_password = doc["wifi_ap_password"] | wifi_ap_password;
        wifi_sta_ssid = doc["wifi_sta_ssid"] | wifi_sta_ssid;
        wifi_sta_password = doc["wifi_sta_password"] | wifi_sta_password;
    };
    // set data according to doc
    void fromWeb(JsonVariant variant)
    {
        // Copy values from the variant to the Config
        if (!variant["hostname"].isNull())
            hostname = variant["hostname"].as<String>();
        if (!variant["wifi_ap_mode"].isNull())
            wifi_ap_mode = variant["wifi_ap_mode"].as<bool>();
        if (!variant["wifi_ap_ssid"].isNull())
            wifi_ap_ssid = variant["wifi_ap_ssid"].as<String>();
        if (!variant["wifi_ap_password"].isNull())
            wifi_ap_password = variant["wifi_ap_password"].as<String>();
        if (!variant["wifi_sta_ssid"].isNull())
            wifi_sta_ssid = variant["wifi_sta_ssid"].as<String>();
        if (!variant["wifi_sta_password"].isNull())
            wifi_sta_password = variant["wifi_sta_password"].as<String>();
    };
};

struct SensorConfig : BaseConfig
{
    // public:
    //  SensorConfig() : BaseConfig(String("config_sensor.json")){}; // constructor
    using BaseConfig::BaseConfig; // Inherit BaseConfig's constructors.
public:
    // data
    String name = "Default Sensor";
    String serial = "";
    float fullrange = 100;
    float sensitivity = 1.5;
    float zerobalance = 0.0;
    String displayunit = "%";

    // create doc from data
    void toDoc(DynamicJsonDocument &doc) const
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
    void fromDoc(DynamicJsonDocument const &doc)
    {
        // Copy values from the JsonDocument to the Config
        name = doc["name"] | name;
        serial = doc["serial"] | serial;
        fullrange = doc["fullrange"] | fullrange;
        sensitivity = doc["sensitivity"] | sensitivity;
        zerobalance = doc["zerobalance"] | zerobalance;
        displayunit = doc["displayunit"] | displayunit;
    };

    // set data according to doc
    void fromWeb(JsonVariant variant)
    {
        // Copy values from the variant to the Config
        if (!variant["name"].isNull())
            name = variant["name"].as<String>();
        if (!variant["serial"].isNull())
            serial = variant["serial"].as<String>();
        if (!variant["fullrange"].isNull())
            fullrange = variant["fullrange"].as<float>();
        if (!variant["sensitivity"].isNull())
            sensitivity = variant["sensitivity"].as<float>();
        if (!variant["zerobalance"].isNull())
            zerobalance = variant["zerobalance"].as<float>();
        if (!variant["displayunit"].isNull())
            displayunit = variant["displayunit"].as<String>();
    };
};