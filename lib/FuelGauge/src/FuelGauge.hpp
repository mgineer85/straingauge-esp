#pragma once

#include <Arduino.h>
#include "Adafruit_MAX1704X.h"

///
namespace FuelGauge
{
    void initialize();
    float getBatteryPercent();
    float getBatteryVoltage();
    void update_loop();
}