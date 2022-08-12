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
    - implement battery IC


*/

#include <ArduinoJson.h>
#include <stdio.h>
#include <Preferences.h>
Preferences main_preferences;

#include "Webservice.hpp"
#include "Loadcell.hpp"
#include "Display.hpp"

/*
 * Events to couple the modules...
 */
// EventSolution1: https://github.com/esp32m/events
// eval: ok, other considered event busses: MicroQT, Automaton, TaskManagerIO, Eventually
#include <DataEvent.hpp>
using namespace esp32m;

ulong updateLastMillis = 0;

void setup()
{
  Serial.begin(115200);
  Wire.begin();

  main_preferences.begin("main-app", false);

  Display::initialize();
  Webservice::initialize();
  Loadcell::initialize();
  FuelGauge::initialize();
  Display::static_content(); // display static content after init all modules
}

void loop()
{
  Loadcell::update_loop();

  // update display+webservice not every loop due to high load.
  if ((millis() - updateLastMillis) > 250)
  {
    updateLastMillis = millis();

    Display::set_variables(Loadcell::getForce(), Loadcell::getReading(), FuelGauge::getBatteryPercent());
    Display::update_loop();

    // events + data
    Webservice::invokeSendEvent("ping", String(millis()));
    Webservice::invokeSendEvent("reading", String(Loadcell::getReading()));
    Webservice::invokeSendEvent("weight", String(Loadcell::getWeight(), 1));
    Webservice::invokeSendEvent("force", String(Loadcell::getForce(), 0));
  }
}
