/*
  Strain Gauge Amplifier
  By: Michael G.
  Date: 2023-10-03
  License: MIT

  Compact mobile strain gauge amplifier with battery

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
    - any use for GPIO13 LED on board?


*/

#include <Arduino.h>
#include <FFat.h>

#include "System.hpp"    // --> g_System
#include "Fuelgauge.hpp" // --> g_Fuelgauge
#include "Loadcell.hpp"  // --> g_Loadcell
#include "Webservice.hpp"
#include "Display.hpp"

#include "Button2.h"
#define BUTTON_PIN 0
Button2 button;

SystemConfig system_config = SystemConfig("system.json");

/*
 * Events to couple the modules...
 */
// EventSolution1: https://github.com/esp32m/events
// eval: ok, other considered event busses: MicroQT, Automaton, TaskManagerIO, Eventually
#include <DataEvent.hpp>
using namespace esp32m;

/////////////////////////////////////////////////////////////////

void pressed(Button2 &btn)
{
  log_i("tare button pressed");

  // send event to inform other modules to action
  Event ev("Loadcell/tare");
  EventManager::instance().publish(ev);
}

void Task_Display(void *pvParameters)
{
  (void)pvParameters;

  while (1) // A Task shall never return or exit.
  {
    Display::update_loop();

    vTaskDelay(250 / portTICK_PERIOD_MS);
  }
}

void Task_Loadcell(void *pvParameters)
{
  (void)pvParameters;

  g_Loadcell.initialize();

  while (1) // A Task shall never return or exit.
  {
    g_Loadcell.update_loop();

    vTaskDelay(1);
  }
}

void Task_Fuelgauge(void *pvParameters)
{
  (void)pvParameters;

  g_Fuelgauge.initialize();

  while (1) // A Task shall never return or exit.
  {
    g_Fuelgauge.update_loop();

    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

void Task_Buttons(void *pvParameters)
{
  (void)pvParameters;

  button.begin(BUTTON_PIN);
  button.setPressedHandler(pressed);

  while (1) // A Task shall never return or exit.
  {
    button.loop();
    vTaskDelay(1);
  }
}

void Task_RegularInfoOut(void *pvParameters)
{
  (void)pvParameters;

  while (1) // A Task shall never return or exit.
  {
    // events + data
    Webservice::invokeSendEvent("ping", String(millis()));
    Webservice::invokeSendEvent("reading", String(g_Loadcell.getReadingRaw()));
    Webservice::invokeSendEvent("force", String(g_Loadcell.getReadingDisplayunitFiltered(), 0));
    Webservice::invokeSendEvent("battery", String(g_Fuelgauge.getBatteryPercent(), 1));

    // send debug information
    Serial.println();
    Serial.print(g_Loadcell.getReadingRaw());
    Serial.print("\t");
    Serial.print(g_Loadcell.getReadingDisplayunitFiltered(), 4);
    Serial.print("\t");
    Serial.print(g_Fuelgauge.getBatteryPercent(), 1);
    Serial.print("\t");
    Serial.print(g_Fuelgauge.getChargeRate(), 1);

    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  log_i("Starting strain gauge box");

  // early init phase
  Display::initialize();
  Display::status_message("display initialized");

  // onetime load system config on start, also setup filesystem+wifi
  g_System.initialize();

  // later init phase
  xTaskCreatePinnedToCore(Task_Buttons, "Task_Buttons", 4096, NULL, 2, NULL, ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(Task_Loadcell, "Task_Loadcell", 4096, NULL, 3, NULL, ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(Task_Fuelgauge, "Task_Fuelgauge", 4096, NULL, 3, NULL, ARDUINO_RUNNING_CORE);
  Webservice::initialize();

  Display::status_message("Ready.");

  xTaskCreatePinnedToCore(Task_Display, "Task_Display", 4096, NULL, 2, NULL, ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(Task_RegularInfoOut, "Task_RegularInfoOut", 4096, NULL, 3, NULL, ARDUINO_RUNNING_CORE);
}

void loop()
{
  // empty
}
