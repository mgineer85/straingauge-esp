#include <Fuelgauge.hpp>

FuelgaugeClass g_Fuelgauge;

FuelgaugeClass::FuelgaugeClass()
{
    // on init construct with default variables
}
void FuelgaugeClass::initialize()
{

    log_i("Fuelgauge init");

    if (!battery_gauge.begin())
    {
        log_e("Couldnt find battery fuel gauge chip?\nMake sure a battery is plugged in! Continue regular checks");
        _batteryConnected = false;
    }
    else
    {
        _batteryConnected = true;

        log_i("Found MAX17048, ID 0x%02hhx", battery_gauge.getChipID());
    }
}

float FuelgaugeClass::getBatteryPercent()
{
    return (_batteryConnected) ? _batteryCellPercent : 0.0;
}

float FuelgaugeClass::getBatteryVoltage()
{
    return (_batteryConnected) ? _batteryVoltage : 0.0;
}

float FuelgaugeClass::getChargeRate()
{
    return (_batteryConnected) ? _chargeRate : 0.0;
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

        _chargeRate = battery_gauge.chargeRate();

        _isCharging = (_chargeRate > 0.0 && _batteryCellPercent < 100) ? true : false;
    }
}
