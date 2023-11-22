/*

*/

#include "Ads1220lib.h"

// Constructor
ADS1220::ADS1220() {}

// Sets up the ADS1220 for basic function
// Returns true upon completion
bool ADS1220::begin(uint8_t DRDY_pin, uint8_t CS_pin, SPIClass &spiPort)
{
  // Get user's options
  _DRDY = DRDY_pin;
  _CS = CS_pin;
  _spiPort = &spiPort;

  _spiPort->begin(_CS);
  _SPISettings = SPISettings(4000000, MSBFIRST, SPI_MODE1);

  pinMode(_CS, OUTPUT);
  pinMode(_DRDY, INPUT);
  digitalWrite(_CS, HIGH);

  // if isnt connected, fail early.
  if (isConnected() == false) // Check for sensor
    return false;

  bool result = true; // Accumulate a result as we do the setup

  result &= reset();                                        // Reset all registers
  result &= setConversionMode(ADS1220_CM_SINGLE_SHOT_MODE); // Power on analog and digital sections of the scale

  return (result);
}

// Returns true if device is present
bool ADS1220::isConnected()
{
  // TODO: add check
  return (true); // All good
}

// Returns true if Cycle Ready bit is set (conversion is complete)
bool ADS1220::available()
{
  return digitalRead(_DRDY) == LOW;
}

bool ADS1220::start_single_shot_measurement()
{
  return command(ADS1220_START);
}

// Resets all registers to Power Of Defaults
bool ADS1220::reset()
{
  bool retval = command(ADS1220_RESET);
  delay(1);
  return retval;
}

bool ADS1220::powerDown()
{
  return command(ADS1220_PWRDOWN);
}

bool ADS1220::setConversionMode(ADS1220_CM_Values cm)
{
  return setBit(ADS1220_CTRL1_CM, ADS1220_CTRL1, cm);
}

bool ADS1220::pgaBypass(ADS1220_PGA_BYPASS_Values pga_bypass)
{
  return setBit(ADS1220_CTRL0_PGA_BYPASS, ADS1220_CTRL0, pga_bypass);
}

bool ADS1220::setSampleRate(ADS1220_DR_Values dr)
{
  uint8_t value = readRegister(ADS1220_CTRL1);
  value &= 0b00011111; // Clear gain bits
  value |= dr << 5;    // Mask in new bits

  return (writeRegister(ADS1220_CTRL1, value));
}

bool ADS1220::setMux(ADS1220_MUX_Values mux)
{
  uint8_t value = readRegister(ADS1220_CTRL0);
  value &= 0b00001111; // Clear gain bits
  value |= mux << 4;   // Mask in new bits

  return (writeRegister(ADS1220_CTRL0, value));
}

bool ADS1220::setGain(ADS1220_GAIN_Values gain)
{
  uint8_t value = readRegister(ADS1220_CTRL0);
  value &= 0b11110001; // Clear gain bits
  value |= gain << 1;  // Mask in new bits

  return (writeRegister(ADS1220_CTRL0, value));
}

// Returns 24-bit reading
// Assumes CR Cycle Ready bit (ADC conversion complete) has been checked to be 1
int32_t ADS1220::getReading()
{

  if (getBit(ADS1220_CTRL1_CM, ADS1220_CTRL1) == ADS1220_CM_SINGLE_SHOT_MODE)
  {
    start_single_shot_measurement();
  }
  while (!available())
  {
  }

  _spiPort->beginTransaction(_SPISettings);
  digitalWrite(_CS, LOW);
  uint32_t valueRaw = (uint32_t)_spiPort->transfer(0x00) << 16; // MSB
  valueRaw |= (uint32_t)_spiPort->transfer(0x00) << 8;          // MidSB
  valueRaw |= (uint32_t)_spiPort->transfer(0x00);               // LSB

  digitalWrite(_CS, HIGH);
  _spiPort->endTransaction();

  // the raw value coming from the ADC is a 24-bit number, so the sign bit now
  // resides on bit 23 (0 is LSB) of the uint32_t container. By shifting the
  // value to the left, I move the sign bit to the MSB of the uint32_t container.
  // By casting to a signed int32_t container I now have properly recovered
  // the sign of the original value
  int32_t valueShifted = (int32_t)(valueRaw << 8);

  // shift the number back right to recover its intended magnitude
  int32_t value = (valueShifted >> 8);

  return (value);
}

// Call when scale is setup, level, at running temperature, with nothing on it
bool ADS1220::tara()
{
  return true;
}

/* *********************************************************
 *  SPI communication methods
 ********************************************************* */

// Mask & set a given bit within a register
bool ADS1220::setBit(uint8_t bitNumber, uint8_t registerAddress, bool value)
{
  if (value)
  {
    return setBit(bitNumber, registerAddress);
  }
  else
  {
    return clearBit(bitNumber, registerAddress);
  }
}

// Mask & set a given bit within a register
bool ADS1220::setBit(uint8_t bitNumber, uint8_t registerAddress)
{
  uint8_t value = readRegister(registerAddress);
  value |= (1 << bitNumber); // Set this bit
  return (writeRegister(registerAddress, value));
}

// Mask & clear a given bit within a register
bool ADS1220::clearBit(uint8_t bitNumber, uint8_t registerAddress)
{
  uint8_t value = readRegister(registerAddress);
  value &= ~(1 << bitNumber); // Set this bit
  return (writeRegister(registerAddress, value));
}

// Return a given bit within a register
bool ADS1220::getBit(uint8_t bitNumber, uint8_t registerAddress)
{
  uint8_t value = readRegister(registerAddress);
  value &= (1 << bitNumber); // Clear all but this bit
  return (value);
}

uint8_t ADS1220::readRegister(uint8_t reg)
{
  uint8_t regValue;

  _spiPort->beginTransaction(_SPISettings);
  digitalWrite(_CS, LOW);
  _spiPort->transfer(ADS1220_RREG | (reg << 2));
  regValue = _spiPort->transfer(0x00);
  digitalWrite(_CS, HIGH);
  _spiPort->endTransaction();

  return regValue;
}

bool ADS1220::writeRegister(uint8_t reg, uint8_t val)
{
  _spiPort->beginTransaction(_SPISettings);
  digitalWrite(_CS, LOW);
  _spiPort->transfer(ADS1220_WREG | (reg << 2));
  _spiPort->transfer(val);
  digitalWrite(_CS, HIGH);
  _spiPort->endTransaction();

  return true; // always return true because SPI errors cannot be detected
}

bool ADS1220::command(uint8_t cmd)
{
  _spiPort->beginTransaction(_SPISettings);
  digitalWrite(_CS, LOW);
  _spiPort->transfer(cmd);
  digitalWrite(_CS, HIGH);
  _spiPort->endTransaction();

  return true; // always return true because SPI errors cannot be detected
}