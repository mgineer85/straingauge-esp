#include "Loadcell.hpp"

#include <RunningMedian.h>

#include <DataEvent.hpp>
using namespace esp32m;

#define AVG_SIZE 8

namespace Loadcell
{
    NAU7802 myScale; // Create instance of the NAU7802 class
    Preferences preferences;
    RunningMedian filterWeight = RunningMedian(AVG_SIZE);

    // Record the current system settings to EEPROM
    void recordSystemSettings(void)
    {
        preferences.putFloat("cal_factor", myScale.getCalibrationFactor());
        preferences.putInt("zero_offset", myScale.getZeroOffset());
    }

    // Reads the current system settings from EEPROM
    // If anything looks weird, reset setting to default value
    void readSystemSettings(void)
    {
        // Pass these values to the library
        myScale.setCalibrationFactor(preferences.getFloat("cal_factor", 1));
        myScale.setZeroOffset(preferences.getInt("zero_offset", 1));
    }

    void initialize()
    {
        preferences.begin("loadcell-module", false);

        if (!myScale.begin())
        {
            Serial.println("Scale not detected. Please check wiring. Retry...");
            delay(1000);
            if (!myScale.begin())
                Serial.println("Scale not detected in second run. Ignoring...");
        }
        readSystemSettings(); // Load zeroOffset and calibrationFactor from EEPROM
        myScale.setZeroOffset(-5446);
        myScale.setCalibrationFactor(4200);

        myScale.setChannel(NAU7802_CHANNEL_1);
        myScale.setSampleRate(NAU7802_SPS_10); // Increase to max sample rate
        myScale.setGain(NAU7802_GAIN_128);
        myScale.setLDO(NAU7802_LDO_3V0);
        myScale.calibrateAFE(); // Re-cal analog front end when we change gain, sample rate, or channel

        Serial.print("Zero offset: ");
        Serial.println(myScale.getZeroOffset());
        Serial.print("Calibration factor: ");
        Serial.println(myScale.getCalibrationFactor());

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
            if (ev->is("Loadcell/calibrateByWeight"))
            {
                Serial.println("Loadcell/calibrateByWeight");
                Serial.println(((DataEvent *)ev)->data());

                cmdCalcCalibrationFactor(((DataEvent *)ev)->data().toFloat());
        } });
    }

    // getter for external readout
    int32_t getReading()
    {
        return myScale.getReading();
    }
    float getWeight()
    {
        return filterWeight.getAverage();
    }
    float getForce()
    {
        return getWeight() * 9.81;
    }

    // commands triggered externally
    void cmdTare()
    {
        myScale.calculateZeroOffset(8);
    }
    void cmdCalcCalibrationFactor(float weightOnScale)
    {
        // step 1: tare
        // step 2: put known weight on scale
        // step 3: calc cal factor
        myScale.calculateCalibrationFactor(weightOnScale);
        // step 4: store cal factor
    }
    void cmdSetCalibrationFactor(float factor)
    {
        myScale.setCalibrationFactor(factor);
    }
    void cmdInternalCalibration()
    {
        myScale.calibrateAFE();
    }
    void cmdPersistPreferences()
    {
        recordSystemSettings();
    }

    void update_loop()
    {
        if (myScale.available() == true)
        {
            float currentWeight = myScale.getWeight(true, 1);
            filterWeight.add(currentWeight);
        }
    }
}