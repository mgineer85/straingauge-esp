/*
  Use the Qwiic Scale to read load cells and scales
  By: Nathan Seidle @ SparkFun Electronics
  Date: March 3rd, 2019
  License: This code is public domain but you buy me a beer if you use this
  and we meet someday (Beerware license).

  This example shows how to setup a scale complete with zero offset (tare),
  and linear calibration.

  If you know the calibration and offset values you can send them directly to
  the library. This is useful if you want to maintain values between power cycles
  in EEPROM or Non-volatile memory (NVM). If you don't know these values then
  you can go through a series of steps to calculate the offset and calibration value.

  Background: The IC merely outputs the raw data from a load cell. For example, the
  output may be 25776 and change to 43122 when a cup of tea is set on the scale.
  These values are unitless - they are not grams or ounces. Instead, it is a
  linear relationship that must be calculated. Remeber y = mx + b?
  If 25776 is the 'zero' or tare state, and 43122 when I put 15.2oz of tea on the
  scale, then what is a reading of 57683 in oz?

  (43122 - 25776) = 17346/15.2 = 1141.2 per oz
  (57683 - 25776) = 31907/1141.2 = 27.96oz is on the scale

  SparkFun labored with love to create this code. Feel like supporting open
  source? Buy a board from SparkFun!
  https://www.sparkfun.com/products/15242

  Hardware Connections:
  Plug a Qwiic cable into the Qwiic Scale and a RedBoard Qwiic
  If you don't have a platform with a Qwiic connection use the SparkFun Qwiic Breadboard Jumper (https://www.sparkfun.com/products/14425)
  Open the serial monitor at 9600 baud to see the output
*/

#include <ArduinoJson.h>
#include <stdio.h>
#include <Preferences.h>
Preferences main_preferences;

#include "Webservice.hpp"
#include "Loadcell.hpp"
#include "Display.hpp"

#include <events.hpp>
using namespace esp32m;
class CustomEvent<typename T>
    : public Event
{
public:
  CustomEvent(T data) : Event("custom"), _data(data) {}
  T data() { return _data; }

private:
  T _data;
};

ulong updateLastMillis = 0;

void onActionTare();
void onActionCalibrateFactor();

void setup()
{
  Serial.begin(115200);

  main_preferences.begin("main-app", false);

  Webservice::initialize();
  Display::initialize();
  Loadcell::initialize();

  // commands
  Webservice::setCallback(Webservice::Tare, &onActionTare);
  Webservice::setCallback(Webservice::CalibrateFactor, &onActionCalibrateFactor);

  EventManager::instance().subscribe([](Event *ev)
                                     {
    if (ev->is("notify"))
    {
    Serial.println("notified!");
    } });
}

void loop()
{
  Loadcell::update_loop();

  // update display+webservice not every loop due to high load.
  if ((millis() - updateLastMillis) > 250)
  {
    updateLastMillis = millis();

    Display::set_variables(Loadcell::getForce(), Loadcell::getReading(), 0);
    Display::update_loop();

    // events + data
    Webservice::invokeSendEvent("ping", String(millis()));
    Webservice::invokeSendEvent("reading", String(Loadcell::getReading()));
    Webservice::invokeSendEvent("weight", String(Loadcell::getWeight(), 1));
    Webservice::invokeSendEvent("force", String(Loadcell::getForce(), 0));
  }
}

void onActionTare()
{
  Loadcell::cmdTare();
  Event ev("notify");
  EventManager::instance().publish(ev);
}

void onActionCalibrateFactor()
{
  float knownWeight = 123; // TODO: wo bekomme ich das her?
  Loadcell::cmdCalcCalibrationFactor(knownWeight);
}