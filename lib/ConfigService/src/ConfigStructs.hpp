#pragma once

#define CONFIG_DIR "/config/"
#define MAX_DOCUMENT_SIZE 1024

#include <Arduino.h>
#include <FFat.h>
#include <ArduinoJson.h>
#include "Adafruit_NAU7802.h"

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

    String serial = "";

    // create doc from data
    void toDoc(DynamicJsonDocument &doc) const
    {
        // Set the values in the document
        doc["hostname"] = hostname;
        doc["wifi_ap_mode"] = wifi_ap_mode;
        doc["wifi_ap_ssid"] = wifi_ap_ssid;
        doc["wifi_ap_password"] = wifi_ap_password;
        doc["serial"] = serial;
    };

    // set data according to doc
    void fromDoc(DynamicJsonDocument const &doc)
    {
        // Copy values from the JsonDocument to the Config
        hostname = doc["hostname"] | hostname;
        wifi_ap_mode = doc["wifi_ap_mode"] | wifi_ap_mode;
        wifi_ap_ssid = doc["wifi_ap_ssid"] | wifi_ap_ssid;
        wifi_ap_password = doc["wifi_ap_password"] | wifi_ap_password;
        serial = doc["serial"] | serial;
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
        if (!variant["serial"].isNull())
            serial = variant["serial"].as<String>();
    };
};

struct SensorConfig : BaseConfig
{
    using BaseConfig::BaseConfig; // Inherit BaseConfig's constructors.
public:
    // data
    String name = "neutral settings";
    String serial = "";
    float fullrange = 1.0;
    float sensitivity = 1.0;
    float zerobalance = 0.0;
    String displayunit = "mV/V";
    uint8_t digits = 4;

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
        doc["digits"] = digits;
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
        digits = doc["digits"] | digits;
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
        if (!variant["digits"].isNull())
            digits = variant["digits"].as<uint8_t>();
    };
};

struct AdcConfig : BaseConfig
{
    using BaseConfig::BaseConfig; // Inherit BaseConfig's constructors.
public:
    // data
    NAU7802_LDOVoltage ldovoltage = NAU7802_3V0;
    NAU7802_Gain gain = NAU7802_GAIN_128;
    NAU7802_SampleRate samplerate = NAU7802_RATE_10SPS;
    float cali_offset = 0.0;
    float cali_gain_factor = 1.0;

    // create doc from data
    void toDoc(DynamicJsonDocument &doc) const
    {
        // Set the values in the document
        doc["ldovoltage"] = ldovoltage;
        doc["gain"] = gain;
        doc["samplerate"] = samplerate;
        doc["cali_offset"] = cali_offset;
        doc["cali_gain_factor"] = cali_gain_factor;
    };

    // set data according to doc
    void fromDoc(DynamicJsonDocument const &doc)
    {
        // Copy values from the JsonDocument to the Config
        ldovoltage = doc["ldovoltage"] | ldovoltage;
        gain = doc["gain"] | gain;
        samplerate = doc["samplerate"] | samplerate;
        cali_offset = doc["cali_offset"] | cali_offset;
        cali_gain_factor = doc["cali_gain_factor"] | cali_gain_factor;
    };

    // set data according to doc
    void fromWeb(JsonVariant variant)
    {
        // Copy values from the variant to the Config
        if (!variant["ldovoltage"].isNull())
            ldovoltage = variant["ldovoltage"].as<NAU7802_LDOVoltage>();
        if (!variant["gain"].isNull())
            gain = variant["gain"].as<NAU7802_Gain>();
        if (!variant["samplerate"].isNull())
            samplerate = variant["samplerate"].as<NAU7802_SampleRate>();
        if (!variant["cali_offset"].isNull())
            cali_offset = variant["cali_offset"].as<float>();
        if (!variant["cali_gain_factor"].isNull())
            cali_gain_factor = variant["cali_gain_factor"].as<float>();
    };
};