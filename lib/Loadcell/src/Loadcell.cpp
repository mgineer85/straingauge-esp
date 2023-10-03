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

    nau7802_adc.setChannel(NAU7802_CHANNEL_1);
    nau7802_adc.setSampleRate(NAU7802_SPS_10); // Increase to max sample rate
    nau7802_adc.setGain(gain_bits);
    nau7802_adc.setLDO(NAU7802_LDO_3V0);

    // Re-cal analog front end when we change gain, sample rate, or channel
    if (!nau7802_adc.calibrateAFE())
        log_e("internal ADC calibration failed!");
    else
        log_i("internal ADC calibration successfull.");

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
}
void LoadcellClass::cbLoadConfiguration(void)
{
    sensor_config.loadConfiguration();
    postConfigChange();
}
void LoadcellClass::postConfigChange(void)
{

    float sensor_scale_factor = ((float)adc_resolution * gain * (sensor_config.sensitivity)) / (1000.0 * sensor_config.fullrange);
    int32_t sensor_zero_balance_raw = (int)round(sensor_config.zerobalance * (float)adc_resolution * gain / 1000.0);

    nau7802_adc.setCalibrationFactor(sensor_scale_factor);
    nau7802_adc.setZeroOffset(sensor_zero_balance_raw);

    log_i("Zero offset: %i", nau7802_adc.getZeroOffset());
    log_i("Calibration factor: %0.2f", nau7802_adc.getCalibrationFactor());

    nau7802_adc.calibrateAFE();
}

// getter for external readout
int32_t LoadcellClass::getReadingRaw()
{
    return nau7802_adc.getReading();
}
float LoadcellClass::getReadingDisplayunitFiltered()
{
    return _readingDisplayunitFiltered.getAverage();
}

// commands triggered externally
void LoadcellClass::cmdZeroOffsetTare()
{
    nau7802_adc.calculateZeroOffset(4); // TODO: needs fix: if 8 averages, Sparkfun lib times out and silently returns 0!

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

void LoadcellClass::update_loop()
{
    if (nau7802_adc.available() == true)
    {
        _readingDisplayunitFiltered.add(nau7802_adc.getWeight(true, 1)); // getWeight means actually receiving the reading divided by the calibration factor to convert to SI-unit. We calibrate for N, so this is it.
    }
}
