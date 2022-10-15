/*
  Strain Gauge Amplifier
  By: Michael Gröne
  Date: 2022-08-10
  License: tbd

  Small mobile strain gauge amplifier with battery

  Features:
    - mobile case 3d printed
    - battery powered, remaining battery display
    - charge by usb c
    - display for measurement data
    - webinterface via AP
      - extended measurement data
      - calibration menu
      - change settings and persist preferences


  BOM:
    - tbd

  TODOs:
    - cleanup/refactor
    - Quasar webinterface
    - test battery interface
    - implement tare button functionality (GPIO0 button) using Button2 library.
    - any use for GPIO13 LED on s2 board?


*/

#include <ArduinoJson.h>
#include <stdio.h>
#include <Preferences.h>
Preferences main_preferences;

#include "Webservice.hpp"
#include "Loadcell.hpp"
#include "Display.hpp"
#include "FuelGauge.hpp"

#include "Button2.h"
#define BUTTON_PIN 0
Button2 button;

/*
 * Events to couple the modules...
 */
// EventSolution1: https://github.com/esp32m/events
// eval: ok, other considered event busses: MicroQT, Automaton, TaskManagerIO, Eventually
#include <DataEvent.hpp>
using namespace esp32m;

ulong updateLastMillisFast = 0;
ulong updateLastMillisSlow = 0;

/////////////////////////////////////////////////////////////////
void pressed(Button2 &btn)
{
  Serial.println("tare button pressed");

  // send event to inform other modules to action
  Event ev("Loadcell/tare");
  EventManager::instance().publish(ev);
}

void setup()
{
  delay(200); // wait for all modules to start up before init communication. without this delay some modules lead to freezing program because the I2C communication fails.
  Serial.begin(115200);

  button.begin(BUTTON_PIN);
  button.setPressedHandler(pressed);

  main_preferences.begin("main-app", false);

  Display::initialize();
  Webservice::initialize();
  Loadcell::initialize();
  FuelGauge::initialize();
  Display::static_content(); // display static content after init all modules

  // after all inits set frequency to
  Wire.setClock(400000U); // 100kHz is default but has issues: https://github.com/espressif/esp-idf/issues/8770. testing 400k now (max of slaves)
}

void loop()
{
  button.loop();
  Loadcell::update_loop();

  // update display+webservice not every loop due to high load.
  if ((millis() - updateLastMillisFast) > 500)
  {
    updateLastMillisFast = millis();

    Display::set_variables(Loadcell::getForce(), Loadcell::getReading(), FuelGauge::getBatteryPercent());
    Display::update_loop();

    // events + data
    Webservice::invokeSendEvent("ping", String(millis()));
    Webservice::invokeSendEvent("reading", String(Loadcell::getReading()));
    Webservice::invokeSendEvent("force", String(Loadcell::getForce(), 0));

    // send debug information
    Serial.println();
    Serial.print(Loadcell::getReading());
    Serial.print("\t");
    Serial.print(Loadcell::getForce(), 0);
  }

  if ((millis() - updateLastMillisSlow) > 4000)
  {
    updateLastMillisSlow = millis();

    FuelGauge::update_loop();

    // events + data
    Webservice::invokeSendEvent("battery", String(FuelGauge::getBatteryPercent(), 1));

    // send some debug information
    Serial.print("\t");
    Serial.print(FuelGauge::getBatteryPercent(), 1);
    Serial.print("\t");
    Serial.print(FuelGauge::getBatteryVoltage(), 3);
  }
}
