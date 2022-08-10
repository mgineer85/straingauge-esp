#pragma once

#include <Arduino.h>
#include <Wire.h>

#include <Preferences.h>
#include <EEPROM.h> //Needed to record user settings
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <Adafruit_I2CDevice.h>
#include "SparkFun_Qwiic_Scale_NAU7802_Arduino_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_NAU8702

///
namespace Loadcell
{

    void initialize();

    void update_loop();

    // getter for external readout
    int32_t getReading();
    float getWeight();
    float getForce();

    // commands triggered externally
    void cmdTare();
    void cmdCalcCalibrationFactor(float weightOnScale);
    void cmdSetCalibrationFactor(float factor);
    void cmdInternalCalibration();
    void cmdPersistPreferences();
}