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
        log_w("Scale not detected. Please check wiring. Retry...");
        delay(1000);
        if (!nau7802_adc.begin())
            log_e("Scale not detected in second run. Ignoring...");
    }

    this->cbLoadConfiguration(); // Load zeroOffset and calibrationFactor from EEPROM

    // register events

    // commands
    EventManager::instance().subscribe([&](Event *ev)
                                       {
            if (ev->is("Loadcell/tare"))
            {
                log_d("Loadcell/tare");

                this->cmdZeroOffsetTare();
            } });

    EventManager::instance().subscribe([&](Event *ev)
                                       {
            if (ev->is("Loadcell/calibrateToKnownValue"))
            {
                log_d("Loadcell/calibrateToKnownValue, value %s", ((DataEvent *)ev)->data());

                this->cmdCalcCalibrationFactor(((DataEvent *)ev)->data().toFloat());
        } });

    EventManager::instance().subscribe([&](Event *ev)
                                       {
            if (ev->is("*/saveconfiguration"))
            {
                log_d("Loadcell/saveconfiguration");

                this->cbSaveConfiguration();
        } });

    EventManager::instance().subscribe([&](Event *ev)
                                       {
            if (ev->is("*/loadconfiguration"))
            {
                log_d("Loadcell/loadconfiguration");

                this->cbLoadConfiguration();
        } });
}

void LoadcellClass::cbSaveConfiguration(void)
{
    sensor_config.saveConfiguration();
    adc_config.saveConfiguration();
}
void LoadcellClass::cbLoadConfiguration(void)
{
    sensor_config.loadConfiguration();
    adc_config.loadConfiguration();
    postConfigChange();
}
void LoadcellClass::postConfigChange(void)
{
    log_i("postConfigChange triggered");

    // reset average
    _readingDisplayunitFiltered.clear();

    sensor_scale_factor = ((float)adc_resolution * (float)(1 << adc_config.gain) * ((sensor_config.sensitivity * adc_config.cali_gain_factor))) / (1000.0 * sensor_config.fullrange);
    sensor_zero_balance_raw = (int)((sensor_config.zerobalance - adc_config.cali_offset) * (float)adc_resolution * (float)(1 << adc_config.gain) / 1000.0);

    log_i("sensor_scale_factor: %0.2f", sensor_scale_factor);
    log_i("sensor_zero_balance_raw: %i", sensor_zero_balance_raw);

    log_i("adc setRate %i", adc_config.samplerate);
    nau7802_adc.setRate(adc_config.samplerate);
    log_i("adc setGain %i", adc_config.gain);
    nau7802_adc.setGain(adc_config.gain);
    log_i("adc setLDO %i", adc_config.ldovoltage);
    nau7802_adc.setLDO(adc_config.ldovoltage);

    // Take 4 readings to flush out readings
    for (uint8_t i = 0; i < 4; i++)
    {
        while (!nau7802_adc.available())
            delay(1);
        nau7802_adc.read();
    }

    // Re-cal analog front end when we change gain, sample rate, or channel
    // removes internal offset and gain error.
    if (!nau7802_adc.calibrate(NAU7802_CALMOD_INTERNAL))
        log_e("NAU7802_CALMOD_INTERNAL calibration failed!");
    else
        log_i("NAU7802_CALMOD_INTERNAL calibration successful.");
}

// getter for external readout
int32_t LoadcellClass::getReadingRaw()
{
    return current_reading_raw;
}
float LoadcellClass::getReadingDisplayunitFiltered()
{
    return _readingDisplayunitFiltered.getAverage();
}

// commands triggered externally
void LoadcellClass::cmdZeroOffsetTare()
{
    sensor_zero_balance_raw = current_reading_raw;

    DataEvent ev("Webservice/sendMessage", "new zero offset: " + String(sensor_zero_balance_raw));
    EventManager::instance().publish(ev);
}
void LoadcellClass::cmdCalcCalibrationFactor(float knownReference)
{
    // step 1: tare
    // step 2: put known weight on scale
    // step 3: calc cal factor
    // TODO: nau7802_adc.calculateCalibrationFactor(knownReference);
    // step 4: store cal factor

    DataEvent ev("Webservice/sendMessage", "new calibraction factor: " + String(sensor_scale_factor));
    EventManager::instance().publish(ev);
}

void LoadcellClass::update_loop()
{
    if (nau7802_adc.available() == true)
    {
        current_reading_raw = nau7802_adc.read();

        // convert to displayunit: y=(x-b)/m
        _readingDisplayunitFiltered.add(((current_reading_raw - sensor_zero_balance_raw) / sensor_scale_factor));
    }
}
