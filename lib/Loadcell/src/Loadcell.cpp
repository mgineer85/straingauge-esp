#include "Loadcell.hpp"
#include <RunningMedian.h>
#include <DataEvent.hpp>
#include <ConfigService.hpp>

using namespace esp32m;
using namespace ConfigService;

#define AVG_SIZE 8

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

namespace Loadcell
{
    SensorConfig sensorConfig = SensorConfig(String("config_sensor.json"));

    // struct SensorConfig
    // {
    //     char name[64];
    //     char serial[32];

    //     float fullrange;
    //     char display_unit[8];
    //     float sensitivity;
    //     float zero_balance;
    // };

    struct AmplifierConfig
    {
        int gain;
        int samplerate;
        int average;
    };

    NAU7802 myScale; // Create instance of the NAU7802 class
    RunningMedian filterForceAverage = RunningMedian(AVG_SIZE);

    const int32_t adc_resolution = 1 << 24;
    const uint8_t gain = 128;
    const uint8_t gain_bits = NAU7802_GAIN_128;

    /* Burster 6005 S/N 668862 */
    // float sensor_fullrange = 5000;
    // float sensor_sensitivity = 1.7026;
    // float sensor_zerobalance = 0.0098;
    // String sensor_displayunit = "N";

    /* Amazon DMS sensor 1000kg */
    // float sensor_fullrange = 1000;
    // float sensor_sensitivity = 2.029289981;
    // float sensor_zerobalance = -0.002957;
    // String sensor_displayunit = "kg";

    // Record the current system settings to EEPROM
    void recordSystemSettings(void)
    {
        ConfigService::saveConfiguration(sensorConfig);
    }

    // Reads the current system settings from EEPROM
    // If anything looks weird, reset setting to default value
    void readSystemSettings(void)
    {
        ConfigService::loadConfiguration(sensorConfig);

        float sensor_scale_factor = ((float)adc_resolution * gain * (sensorConfig.sensitivity)) / (1000.0 * sensorConfig.fullrange);
        int32_t sensor_zero_balance_raw = (int)round(sensorConfig.zerobalance * (float)adc_resolution * gain / 1000.0);

        myScale.setCalibrationFactor(sensor_scale_factor);
        myScale.setZeroOffset(sensor_zero_balance_raw);

        Serial.print("Zero offset: ");
        Serial.println(myScale.getZeroOffset());
        Serial.print("Calibration factor: ");
        Serial.println(myScale.getCalibrationFactor());
    }

    void initialize()
    {
        if (!myScale.begin())
        {
            Serial.println("Scale not detected. Please check wiring. Retry...");
            delay(1000);
            if (!myScale.begin())
                Serial.println("Scale not detected in second run. Ignoring...");
        }

        readSystemSettings(); // Load zeroOffset and calibrationFactor from EEPROM

        myScale.setChannel(NAU7802_CHANNEL_1);
        myScale.setSampleRate(NAU7802_SPS_10); // Increase to max sample rate
        myScale.setGain(gain_bits);
        myScale.setLDO(NAU7802_LDO_3V0);

        // Re-cal analog front end when we change gain, sample rate, or channel
        if (!myScale.calibrateAFE())
            Serial.println("internal ADC calibration failed!");
        else
            Serial.println("internal ADC calibration successfull.");

        // register events
        EventManager::instance().subscribe([](Event *ev)
                                           {
            if (ev->is("Loadcell/setCalibrationFactor"))
            {
                Serial.println("Loadcell/setCalibrationFactor");
                Serial.println(((DataEvent *)ev)->data());

                cmdSetCalibrationFactor(((DataEvent *)ev)->data().toFloat());
            } });

        // commands
        EventManager::instance().subscribe([](Event *ev)
                                           {
            if (ev->is("Loadcell/tare"))
            {
                Serial.println("Loadcell/tare");

                cmdTare();
            } });

        EventManager::instance().subscribe([](Event *ev)
                                           {
            if (ev->is("Loadcell/calibrateToKnownValue"))
            {
                Serial.println("Loadcell/calibrateToKnownValue");
                Serial.println(((DataEvent *)ev)->data());

                cmdCalcCalibrationFactor(((DataEvent *)ev)->data().toFloat());
        } });

        EventManager::instance().subscribe([](Event *ev)
                                           {
            if (ev->is("*/savePrefs"))
            {
                Serial.println("Loadcell/savePrefs");

                recordSystemSettings();
        } });

        EventManager::instance().subscribe([](Event *ev)
                                           {
            if (ev->is("*/loadPrefs"))
            {
                Serial.println("Loadcell/loadPrefs");

                readSystemSettings();
        } });
    }

    // getter for external readout
    int32_t getReading()
    {
        return myScale.getReading();
    }
    float getForce()
    {
        return filterForceAverage.getAverage();
    }

    // commands triggered externally
    void cmdTare()
    {
        myScale.calculateZeroOffset(8);

        DataEvent ev("Webservice/sendMessage", "new zero offset: " + String(myScale.getZeroOffset()));
        EventManager::instance().publish(ev);
    }
    void cmdCalcCalibrationFactor(float weightOnScale)
    {
        // step 1: tare
        // step 2: put known weight on scale
        // step 3: calc cal factor
        myScale.calculateCalibrationFactor(weightOnScale);
        // step 4: store cal factor

        DataEvent ev("Webservice/sendMessage", "new calibraction factor: " + String(myScale.getCalibrationFactor()));
        EventManager::instance().publish(ev);
    }
    void cmdSetCalibrationFactor(float factor)
    {
        myScale.setCalibrationFactor(factor);
    }
    void cmdInternalCalibration()
    {
        myScale.calibrateAFE();

        DataEvent ev("Webservice/sendMessage", String("internal calibration finished"));
        EventManager::instance().publish(ev);
    }

    void update_loop()
    {
        if (myScale.available() == true)
        {
            filterForceAverage.add(myScale.getWeight(true, 1)); // getWeight means actually receiving the reading divided by the calibration factor to convert to SI-unit. We calibrate for N, so this is it.
        }
    }
}