#pragma once

#include <Arduino.h>
#include "Adafruit_LC709203F.h"

///
namespace FuelGauge
{
    void initialize();
    float getBatteryPercent();
    float getBatteryVoltage();
    void update_loop();
}