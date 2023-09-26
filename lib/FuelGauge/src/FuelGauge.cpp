#include <FuelGauge.hpp>

Adafruit_MAX17048 maxlipo;

namespace FuelGauge
{

    Adafruit_MAX17048 battery_gauge;

    bool _batteryConnected = false;
    float _batteryCellPercent = 0;
    float _batteryVoltage = 0;

    ///
    void initialize()
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
            Serial.println(maxlipo.getChipID(), HEX);
        }
    }

    float getBatteryPercent()
    {
        return (_batteryConnected) ? _batteryCellPercent : 0;
    }

    float getBatteryVoltage()
    {
        return (_batteryConnected) ? _batteryVoltage : 0;
    }

    void update_loop()
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
        }
    }

}
