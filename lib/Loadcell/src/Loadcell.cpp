#include <Loadcell.hpp>

LoadcellClass g_Loadcell;

LoadcellClass::LoadcellClass()
{
    // on init construct with default variables
}

// Record the current system settings to EEPROM
void LoadcellClass::initialize(void)
{

    if (!ads1220_adc.begin(14, 8))
    {
        log_w("ADC not detected. Please check wiring. Retry...");
        delay(1000);
        if (!ads1220_adc.begin(14, 8))
            log_e("ADC not detected in second run. ERROR...");
    }
    log_i("ADC detected.");

    // setup strain gauge config
    ads1220_adc.setConversionMode(ADS1220_CM_CONTINUOUS_CONVERSION_MODE);
    ads1220_adc.setMux(ADS1220_MUX_AIN1_AIN2);
    ads1220_adc.setVoltageReference(ADS1220_VREF_EXTERNAL_REFP1_REFN1);
    ads1220_adc.setGain(ADS1220_GAIN_128);

    // optimized for lowest noise
    ads1220_adc.setOperatingMode(ADS1220_MODE_TURBO);
    ads1220_adc.setDataRate(ADS1220_DR_NORMAL20_DUTY5_TURBO40); // slowest turbo mode. turbo mode and average 4 is lower noise than normal mode average 2.
    ads1220_adc.setFirFilter(ADS1220_FIR_50_60_REJECTION_OFF);  // yes, noise without is lower.

    // offset calibration
    ads1220_adc.internalCalibration();

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

    sensor_scale_factor = ((float)adc_resolution * (float)(1 << adc_config.gain) * ((sensor_config.sensitivity * adc_config.cali_gain_factor))) / (1000.0 * sensor_config.fullrange * 2.0);
    sensor_zero_balance_raw = (int)((sensor_config.zerobalance - adc_config.cali_offset) * (float)adc_resolution * (float)(1 << adc_config.gain) / 1000.0);

    log_i("sensor_scale_factor: %0.2f", sensor_scale_factor);
    log_i("sensor_zero_balance_raw: %i", sensor_zero_balance_raw);

    log_i("adc operating mode %i", adc_config.mode);
    ads1220_adc.setOperatingMode(adc_config.mode);
    log_i("adc setDataRate %i", adc_config.datarate);
    ads1220_adc.setDataRate(adc_config.datarate);
    log_i("adc setGain %i", adc_config.gain);
    ads1220_adc.setGain(adc_config.gain);
    log_i("adc averagereadings will be %i", adc_config.averagereadings);

    // Re-cal analog front end when we change gain, sample rate, or channel
    // removes internal offset and gain error.
    if (!ads1220_adc.internalCalibration())
        log_e("ads1220_adc calibration failed!");
    else
        log_i("ads1220_adc calibration successful.");
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
    // TODO: ads1220_adc.calculateCalibrationFactor(knownReference);
    // step 4: store cal factor

    DataEvent ev("Webservice/sendMessage", "new calibraction factor: " + String(sensor_scale_factor));
    EventManager::instance().publish(ev);
}

void LoadcellClass::update_loop()
{

    current_reading_raw = ads1220_adc.getAverageReading(adc_config.averagereadings);

    // convert to displayunit: y=(x-b)/m
    _readingDisplayunitFiltered.add(((current_reading_raw - sensor_zero_balance_raw) / sensor_scale_factor));
}
