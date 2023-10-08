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
        log_e("Couldnt find battery fuel gauge chip! Gauge display disabled.");
        _batteryGaugeAvail = false;
    }
    else
    {
        log_i("Found MAX17048, ID 0x%02hhx", battery_gauge.getChipID());
        _batteryGaugeAvail = true;
    }
}

bool FuelgaugeClass::getGaugeAvailable()
{
    return _batteryGaugeAvail;
}

float FuelgaugeClass::getBatteryPercent()
{
    return (_batteryGaugeAvail) ? _batteryCellPercent : 0.0;
}

float FuelgaugeClass::getBatteryVoltage()
{
    return (_batteryGaugeAvail) ? _batteryVoltage : 0.0;
}

float FuelgaugeClass::getChargeRate()
{
    return (_batteryGaugeAvail) ? _chargeRate : 0.0;
}

void FuelgaugeClass::update_loop()
{
    if (_batteryGaugeAvail)
    {
        _batteryCellPercent = battery_gauge.cellPercent();
        _batteryCellPercent = _batteryCellPercent < 0 ? 0 : _batteryCellPercent;     // limit to 0%
        _batteryCellPercent = _batteryCellPercent > 100 ? 100 : _batteryCellPercent; // limit to 100%

        _batteryVoltage = battery_gauge.cellVoltage();

        _chargeRate = battery_gauge.chargeRate();
    }
}
