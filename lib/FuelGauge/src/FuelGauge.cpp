#include <FuelGauge.hpp>

///
namespace FuelGauge
{

    Adafruit_LC709203F lc;

    bool _batteryConnected = false;

    ///
    void initialize()
    {
        Serial.println("\nFuel gauge LC709203F init");

        if (!lc.begin())
        {
            Serial.println(F("Couldnt find LC709203F?\nMake sure a battery is plugged in! Continue regular checks"));
            _batteryConnected = false;
        }

        // has to be set, but not used
        lc.setThermistorB(3950);

        // use 1000mAh packages for product
        lc.setPackSize(LC709203F_APA_1000MAH);
    }

    uint8_t getBatteryPercent()
    {
        return (_batteryConnected) ? (uint8_t)(lc.cellPercent()) : 0;
    }

    void update_loop()
    {
        if (!_batteryConnected)
        {
            if (lc.begin())
            {
                _batteryConnected = true;
            }
            else
            {
                _batteryConnected = false;
            }
        }
    }

}