#pragma once

#include <Arduino.h>
#include "Adafruit_LC709203F.h"

///
namespace FuelGauge
{
    void initialize();
    uint8_t getBatteryPercent();
    void update_loop();
}