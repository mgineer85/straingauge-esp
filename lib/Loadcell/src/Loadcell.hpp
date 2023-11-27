#pragma once

#define AVG_SIZE 8
#define ADC_RESOLUTION 24

#include <Arduino.h>
#include <RunningMedian.h>
#include <DataEvent.hpp>
#include <Ads1220lib.h>
#include <ConfigStructs.hpp>

using namespace esp32m;

class LoadcellClass
{
private:
    // Create instance of the ADS1220 class
    ADS1220 ads1220_adc = ADS1220();

    // settings, constants, ...
    const int32_t adc_resolution = 1 << ADC_RESOLUTION;

    // temporary results to convert readings to displayunit
    float sensor_scale_factor = 0;
    int32_t sensor_zero_balance_raw = 0;

    // readings and converted readings
    int32_t current_reading_raw = 0;
    RunningMedian _readingDisplayunitFiltered = RunningMedian(AVG_SIZE); // additional display median

public:
    SensorConfig sensor_config = SensorConfig("sensor.json");
    AdcConfig adc_config = AdcConfig("adc.json");

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
