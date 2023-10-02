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
#include <FFat.h>

#include "utils/filesystem.hpp"
#include "ConfigService.hpp" // --> g_Config
#include "Fuelgauge.hpp"     // --> g_Fuelgauge
#include "Loadcell.hpp"      // --> g_Loadcell
#include "Webservice.hpp"
#include "Display.hpp"

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

/////////////////////////////////////////////////////////////////

void pressed(Button2 &btn)
{
  Serial.println("tare button pressed");

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
    Webservice::invokeSendEvent("reading", String(g_Loadcell.getReading()));
    Webservice::invokeSendEvent("force", String(g_Loadcell.getForce(), 0));
    Webservice::invokeSendEvent("battery", String(g_Fuelgauge.getBatteryPercent(), 1));

    // send debug information
    Serial.println();
    Serial.print(g_Loadcell.getReading());
    Serial.print("\t");
    Serial.print(g_Loadcell.getForce(), 0);
    Serial.print("\t");
    Serial.print(g_Fuelgauge.getBatteryPercent(), 1);
    Serial.print("\t");
    Serial.print(g_Fuelgauge.getIsCharging());

    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void setup()
{
  // delay(200); // wait for all modules to start up before init communication. without this delay some modules lead to freezing program because the I2C communication fails.
  Serial.begin(115200);

  // early init phase
  Display::initialize();
  Display::status_message("display initialized");

  // init filesystem FFAT
  if (!FFat.begin(false, ""))
  {
    Display::status_message("Error mounting filesystem! Halted.");
    while (true)
      ;
  }
  else
  {
    Display::status_message("filesystem mounted successfully");
  }

  Serial.println("Files on FFat:");
  printDirectory(FFat.open("/"));

  // g_MyConfigInstance.printsomevarsfortesting(); // works nice
  // TODO: loadConfiguration(systemConfig);

  // setup WiFi before webservice
  Display::status_message("WiFi connecting...");
  WiFi.mode(WIFI_STA);
  WiFi.hostname(g_Config.system_config.hostname);
  WiFi.begin(g_Config.system_config.wifi_ssid, g_Config.system_config.wifi_password);
  Serial.print("Connect to WiFi, SSID: ");
  Serial.println(g_Config.system_config.wifi_ssid);
  if (WiFi.waitForConnectResult(10000UL) != WL_CONNECTED)
  {
    Display::status_message("WiFi Failed! Halted.");
    while (true)
      ;
  }

  Display::status_message("WiFi connected.");

  // later init phase
  xTaskCreatePinnedToCore(Task_Buttons, "Task_Buttons", 4096, NULL, 2, NULL, ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(Task_Loadcell, "Task_Loadcell", 4096, NULL, 3, NULL, ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(Task_Fuelgauge, "Task_Fuelgauge", 4096, NULL, 3, NULL, ARDUINO_RUNNING_CORE);
  Webservice::initialize();
  Display::static_content(); // display static content after init all modules
  xTaskCreatePinnedToCore(Task_Display, "Task_Display", 4096, NULL, 2, NULL, ARDUINO_RUNNING_CORE);

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Hostname: ");
  Serial.println(WiFi.getHostname());

  Display::status_message("Ready.");

  // after all inits set frequency to
  Wire.setClock(400000U); // 100kHz is default but has issues: https://github.com/espressif/esp-idf/issues/8770. testing 400k now (max of slaves)

  xTaskCreatePinnedToCore(Task_RegularInfoOut, "Task_RegularInfoOut", 4096, NULL, 3, NULL, ARDUINO_RUNNING_CORE);
}

void loop()
{
  // empty
}
