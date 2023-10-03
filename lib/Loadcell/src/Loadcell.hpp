#pragma once

#define AVG_SIZE 8

#include <Arduino.h>
#include "SparkFun_Qwiic_Scale_NAU7802_Arduino_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_NAU8702
#include <RunningMedian.h>
#include <DataEvent.hpp>
#include <ConfigStructs.hpp>
// #include <Adafruit_NAU7802.h>

using namespace esp32m;

class LoadcellClass
{
private:
    struct AmplifierConfig
    {
        int gain;
        int samplerate;
        int average;
    };

    NAU7802 nau7802_adc; // Create instance of the NAU7802 class
    RunningMedian _readingDisplayunitFiltered = RunningMedian(AVG_SIZE);

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

public:
    SensorConfig sensor_config = SensorConfig("sensor.json");

    LoadcellClass();

    void initialize();
    void update_loop();

    // getter for external readout
    int32_t getReadingRaw();
    float getReadingDisplayunitFiltered();

    // commands triggered externally
    void cmdZeroOffsetTare();
    void cmdCalcCalibrationFactor(float knownReference);

    void cbSaveConfiguration(void);
    void cbLoadConfiguration(void);
    void postConfigChange(void);
};

extern LoadcellClass g_Loadcell;
