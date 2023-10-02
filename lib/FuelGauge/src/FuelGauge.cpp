#include <Fuelgauge.hpp>

#define IS_CHARGING_THRESHOLD 0.2

FuelgaugeClass g_Fuelgauge;

FuelgaugeClass::FuelgaugeClass()
{
    // on init construct with default variables
}
void FuelgaugeClass::initialize()
{

    Serial.println("\nFuel gauge init");

    if (!battery_gauge.begin())
    {
        Serial.println(F("Couldnt find battery fuel gauge chip?\nMake sure a battery is plugged in! Continue regular checks"));
        _batteryConnected = false;
    }
    else
    {
        _batteryConnected = true;

        Serial.print(F("Found MAX17048"));
        Serial.print(F(" with Chip ID: 0x"));
        Serial.println(battery_gauge.getChipID(), HEX);
    }
}

float FuelgaugeClass::getBatteryPercent()
{
    return (_batteryConnected) ? _batteryCellPercent : 0;
}

float FuelgaugeClass::getBatteryVoltage()
{
    return (_batteryConnected) ? _batteryVoltage : 0;
}

bool FuelgaugeClass::getIsCharging()
{
    return (_batteryConnected) ? _isCharging : false;
}

void FuelgaugeClass::update_loop()
{
    if (!_batteryConnected)
    {
        if (battery_gauge.begin())
        {
            _batteryConnected = true;
        }
        else
        {
            _batteryConnected = false;
        }
    }
    else
    {
        _batteryCellPercent = battery_gauge.cellPercent();
        _batteryCellPercent = _batteryCellPercent < 0 ? 0 : _batteryCellPercent;     // limit to 0%
        _batteryCellPercent = _batteryCellPercent > 100 ? 100 : _batteryCellPercent; // limit to 100%

        _batteryVoltage = battery_gauge.cellVoltage();

        _isCharging = (battery_gauge.chargeRate() >= IS_CHARGING_THRESHOLD) ? true : false;
    }
}
