#include <Webservice.hpp>
#include <ArduinoJson.h>
#include "SPIFFS.h"
#include <events.hpp>

using namespace esp32m;

namespace Webservice
{
    AsyncWebServer server(80);
    AsyncEventSource events("/events");

    const char *ssid = "chumbawumba";
    const char *password = "Schneepflug";

    const char *PARAM_MESSAGE = "message";

    // modular pattern from here: https://github.com/LuckyResistor/guide-modular-firmware/blob/master/fade_demo_08/fade_demo_08.ino
    // maybe there is better, but works for now.
    const Actions cActions[] = {
        Tare,
        LoadPersistPrefs,
        SavePersistPrefs,
        CalibrateFactor};
    Function gCallback[4];
    void setCallback(Actions action, Function fn)
    {
        gCallback[static_cast<uint8_t>(action)] = fn;
    }

    void notFound(AsyncWebServerRequest *request)
    {
        request->send(404, "text/plain", "Not found");
    }

    void initialize()
    {
        // null callbacks on init
        memset(gCallback, 0, sizeof gCallback);

        EventManager::instance().subscribe([](Event *ev)
                                           {
    if (ev->is("notify"))
    {
      Serial.println("notified!");
    } });

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
            if (request->hasParam("action", true)) {
                action = request->getParam("action", true)->value();
            } else {
                action = "invalid action! -> send 404!? or other error";
            }
            Serial.print("received cmd: ");
            Serial.println(action);

            if(action=="tare") {
                if (gCallback[Actions::Tare] != nullptr)
                {
                    gCallback[Actions::Tare]();
                }
            } else if(action=="calibratefactor") {
                if (gCallback[Actions::CalibrateFactor] != nullptr)
                {
                    gCallback[Actions::CalibrateFactor]();
                }
            }  //TODO other callbacks
            else {
                //error, invalid action
            }

            request->send(200,"text/plain","OK"); });

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
    }

    void invokeSendEvent(String event, String value)
    {
        events.send(value.c_str(), event.c_str(), millis());
    }

}