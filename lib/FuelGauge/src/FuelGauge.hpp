#pragma once

#include <Arduino.h>
#include "Adafruit_MAX1704X.h"

class FuelgaugeClass
{
private:
    Adafruit_MAX17048 battery_gauge;

    bool _batteryGaugeAvail = false;
    float _batteryCellPercent = 0;
    float _batteryVoltage = 0;
    float _chargeRate = 0;

public:
    FuelgaugeClass();

    void initialize();
    void update_loop();
    bool getGaugeAvailable();

    float getBatteryPercent();
    float getBatteryVoltage();
    float getChargeRate();
};

extern FuelgaugeClass g_Fuelgauge;
