#include "Loadcell.hpp"

#include <RunningMedian.h>

#include <DataEvent.hpp>
using namespace esp32m;

#define AVG_SIZE 8

namespace Loadcell
{
    NAU7802 myScale; // Create instance of the NAU7802 class
    Preferences preferences;
    RunningMedian filterForceAverage = RunningMedian(AVG_SIZE);

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
        Serial.println("loadcell readSystemSettings");

        myScale.setCalibrationFactor(preferences.getFloat("cal_factor", 2461));
        myScale.setZeroOffset(preferences.getInt("zero_offset", 0));

        Serial.print("Zero offset: ");
        Serial.println(myScale.getZeroOffset());
        Serial.print("Calibration factor: ");
        Serial.println(myScale.getCalibrationFactor());
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

        myScale.setChannel(NAU7802_CHANNEL_1);
        myScale.setSampleRate(NAU7802_SPS_10); // Increase to max sample rate
        myScale.setGain(NAU7802_GAIN_128);
        myScale.setLDO(NAU7802_LDO_3V0);
        myScale.calibrateAFE(); // Re-cal analog front end when we change gain, sample rate, or channel

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
    void cmdPersistPreferences()
    {
        recordSystemSettings();
    }

    void update_loop()
    {
        if (myScale.available() == true)
        {
            filterForceAverage.add(myScale.getWeight(true, 1)); // getWeight means actually receiving the reading divided by the calibration factor to convert to SI-unit. We calibrate for N, so this is it.
        }
    }
}