#include <Loadcell.hpp>

LoadcellClass g_Loadcell;

LoadcellClass::LoadcellClass()
{
    // on init construct with default variables
}

// Record the current system settings to EEPROM
void LoadcellClass::initialize(void)
{

    if (!nau7802_adc.begin())
    {
        Serial.println("Scale not detected. Please check wiring. Retry...");
        delay(1000);
        if (!nau7802_adc.begin())
            Serial.println("Scale not detected in second run. Ignoring...");
    }

    this->readSystemSettings(); // Load zeroOffset and calibrationFactor from EEPROM

    nau7802_adc.setChannel(NAU7802_CHANNEL_1);
    nau7802_adc.setSampleRate(NAU7802_SPS_10); // Increase to max sample rate
    nau7802_adc.setGain(gain_bits);
    nau7802_adc.setLDO(NAU7802_LDO_3V0);

    // Re-cal analog front end when we change gain, sample rate, or channel
    if (!nau7802_adc.calibrateAFE())
        Serial.println("internal ADC calibration failed!");
    else
        Serial.println("internal ADC calibration successfull.");

    // register events
    EventManager::instance().subscribe([&](Event *ev)
                                       {
            if (ev->is("Loadcell/setCalibrationFactor"))
            {
                Serial.println("Loadcell/setCalibrationFactor");
                Serial.println(((DataEvent *)ev)->data());

                this->cmdSetCalibrationFactor(((DataEvent *)ev)->data().toFloat());
            } });

    // commands
    EventManager::instance().subscribe([&](Event *ev)
                                       {
            if (ev->is("Loadcell/tare"))
            {
                Serial.println("Loadcell/tare");

                this->cmdTare();
            } });

    EventManager::instance().subscribe([&](Event *ev)
                                       {
            if (ev->is("Loadcell/calibrateToKnownValue"))
            {
                Serial.println("Loadcell/calibrateToKnownValue");
                Serial.println(((DataEvent *)ev)->data());

                this->cmdCalcCalibrationFactor(((DataEvent *)ev)->data().toFloat());
        } });

    EventManager::instance().subscribe([&](Event *ev)
                                       {
            if (ev->is("*/saveconfiguration"))
            {
                Serial.println("Loadcell/saveconfiguration");

                this->recordSystemSettings();
        } });

    EventManager::instance().subscribe([&](Event *ev)
                                       {
            if (ev->is("*/loadconfiguration"))
            {
                Serial.println("Loadcell/loadconfiguration");

                this->readSystemSettings();
        } });
}

// Record the current system settings to EEPROM
void LoadcellClass::recordSystemSettings(void)
{
    // TODO: ConfigService::saveConfiguration(sensorConfig);
}

// Reads the current system settings from EEPROM
// If anything looks weird, reset setting to default value
void LoadcellClass::readSystemSettings(void)
{
    // TODO: ConfigService::loadConfiguration(sensorConfig);

    float sensor_scale_factor = ((float)adc_resolution * gain * (g_Config.sensor_config.sensitivity)) / (1000.0 * g_Config.sensor_config.fullrange);
    int32_t sensor_zero_balance_raw = (int)round(g_Config.sensor_config.zerobalance * (float)adc_resolution * gain / 1000.0);

    nau7802_adc.setCalibrationFactor(sensor_scale_factor);
    nau7802_adc.setZeroOffset(sensor_zero_balance_raw);

    Serial.print("Zero offset: ");
    Serial.println(nau7802_adc.getZeroOffset());
    Serial.print("Calibration factor: ");
    Serial.println(nau7802_adc.getCalibrationFactor());
}

// getter for external readout
int32_t LoadcellClass::getReading()
{
    return nau7802_adc.getReading();
}
float LoadcellClass::getForce()
{
    return filterForceAverage.getAverage();
}

// commands triggered externally
void LoadcellClass::cmdTare()
{
    nau7802_adc.calculateZeroOffset(8);

    DataEvent ev("Webservice/sendMessage", "new zero offset: " + String(nau7802_adc.getZeroOffset()));
    EventManager::instance().publish(ev);
}
void LoadcellClass::cmdCalcCalibrationFactor(float weightOnScale)
{
    // step 1: tare
    // step 2: put known weight on scale
    // step 3: calc cal factor
    nau7802_adc.calculateCalibrationFactor(weightOnScale);
    // step 4: store cal factor

    DataEvent ev("Webservice/sendMessage", "new calibraction factor: " + String(nau7802_adc.getCalibrationFactor()));
    EventManager::instance().publish(ev);
}
void LoadcellClass::cmdSetCalibrationFactor(float factor)
{
    nau7802_adc.setCalibrationFactor(factor);
}
void LoadcellClass::cmdInternalCalibration()
{
    nau7802_adc.calibrateAFE();

    DataEvent ev("Webservice/sendMessage", String("internal calibration finished"));
    EventManager::instance().publish(ev);
}

void LoadcellClass::update_loop()
{
    if (nau7802_adc.available() == true)
    {
        filterForceAverage.add(nau7802_adc.getWeight(true, 1)); // getWeight means actually receiving the reading divided by the calibration factor to convert to SI-unit. We calibrate for N, so this is it.
    }
}
