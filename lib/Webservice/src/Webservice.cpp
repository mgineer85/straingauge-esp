#include <Webservice.hpp>
#include <ArduinoJson.h>
#include "SPIFFS.h"
#include <DataEvent.hpp>
using namespace esp32m;

namespace Webservice
{
    AsyncWebServer server(80);
    AsyncEventSource events("/events");

    const char *ssid = "chumbawumba";
    const char *password = "Schneepflug";

    const char *PARAM_MESSAGE = "message";

    void notFound(AsyncWebServerRequest *request)
    {
        request->send(404, "text/plain", "Not found");
    }

    void initialize()
    {
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
        if (WiFi.waitForConnectResult() != WL_CONNECTED)
        {
            Serial.printf("WiFi Failed!\n");
            return;
        }

        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.print("Hostname: ");
        Serial.println(WiFi.getHostname());

        if (!SPIFFS.begin(true))
        {
            Serial.println("An Error has occurred while mounting SPIFFS");
        }

        // attach filesystem root at URL /fs
        server.serveStatic("/", SPIFFS, "/web/")
            .setDefaultFile("index.html");

        // Initialize webserver URLs
        server.on("/api/wifi-info", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
            AsyncResponseStream *response = request->beginResponseStream("application/json");
            DynamicJsonDocument json(1024);
            json["status"] = "ok";
            json["ssid"] = WiFi.SSID();
            json["ip"] = WiFi.localIP().toString();
            json["hostname"] = WiFi.getHostname();
            serializeJson(json, *response);
            request->send(response); });

        // Send a POST request to <IP>/post with a form field message set to <message>
        server.on("/api/cmd", HTTP_POST, [](AsyncWebServerRequest *request)
                  {
            String action;
            if (request->hasParam("action", true))
            {
                action = request->getParam("action", true)->value();
            }
            else
            {
                request->send(400, "text/plain", "action missing");
                return;
            }
            Serial.print("received cmd action: ");
            Serial.println(action);

            if (action == "tare")
            {
                // send event to inform other modules to action
                Event ev("Loadcell/tare");
                EventManager::instance().publish(ev);

                request->send(200, "text/plain", "OK");
            }
            else if (action == "calibrateToKnownValue")
            {

                if (request->hasParam("knownValue", true))
                {
                    String knownValue = request->getParam("knownValue", true)->value();

                    // send event to inform other modules to action
                    DataEvent ev("Loadcell/calibrateToKnownValue", knownValue);
                    EventManager::instance().publish(ev);

                    request->send(200, "text/plain", "OK");
                }
                else
                {
                    request->send(400, "text/plain", "parameter missing");
                    return;
                }
            }
            else if (action == "setCalibrationFactor")
            {
                if (request->hasParam("calibrationFactor", true))
                {
                    String calibrationFactor = request->getParam("calibrationFactor", true)->value();

                    // send event to inform other modules to action
                    DataEvent ev("Loadcell/setCalibrationFactor", calibrationFactor);
                    EventManager::instance().publish(ev);

                    request->send(200, "text/plain", "OK");
                }
                else
                {
                    request->send(400, "text/plain", "parameter missing");
                    return;
                }
            }
            else if (action == "savePreferences")
            {

                // send event to inform other modules to action
                Event ev("*/savePrefs");
                EventManager::instance().publish(ev);

                request->send(200, "text/plain", "OK");
                        }
            else if (action == "loadPreferences")
            {

                // send event to inform other modules to action
                Event ev("*/loadPrefs");
                EventManager::instance().publish(ev);

                request->send(200, "text/plain", "OK");
                        }
            else
            {
                Serial.println("unknown cmd action");
                request->send(400, "text/plain", "unknown cmd action");
                return;
            } });

        // SSE ......
        events.onConnect([](AsyncEventSourceClient *client)
                         {
            if(client->lastId()){
                Serial.printf("Client reconnected! Last message ID that it gat is: %u\n", client->lastId());
            }
            
            client->send("init event session",NULL, millis(), 1000); });

        server.addHandler(&events);

        server.onNotFound(notFound);

        server.begin();

        EventManager::instance().subscribe([](Event *ev)
                                           {
            if (ev->is("Webservice/sendMessage"))
            {
                Serial.println("Webservice/sendMessage");
                Serial.println(((DataEvent *)ev)->data());

                invokeSendEvent("message",((DataEvent *)ev)->data());
        } });
    }

    void invokeSendEvent(String event, String value)
    {
        events.send(value.c_str(), event.c_str(), millis());
    }
}
