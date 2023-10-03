#pragma once

#include <Arduino.h>
#include "Adafruit_MAX1704X.h"

class FuelgaugeClass
{
private:
    Adafruit_MAX17048 battery_gauge;

    bool _batteryConnected = false;
    float _batteryCellPercent = 0;
    float _batteryVoltage = 0;
    float _chargeRate = 0;
    bool _isCharging = false;

public:
    FuelgaugeClass();

    void initialize();
    void update_loop();

    float getBatteryPercent();
    float getBatteryVoltage();
    float getChargeRate();
    bool getIsCharging();
};

extern FuelgaugeClass g_Fuelgauge;
