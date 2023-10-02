#include <Webservice.hpp>

using namespace esp32m;

namespace Webservice
{
    AsyncWebServer server(80);
    AsyncEventSource events("/events");

    void route_webapp_init()
    {

        // attach filesystem root at URL /fs
        server.serveStatic("/", FFat, "/q/")
            .setDefaultFile("index.html");
    }

    void route_status_init()
    {

        // gather information about connection status
        server.on("/status/wifi-info", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
            AsyncResponseStream *response = request->beginResponseStream("application/json");
            DynamicJsonDocument json(1024);
            json["wifi_status"] = WiFi.status();
            json["wifi_ssid"] = WiFi.SSID();
            json["wifi_ip"] = WiFi.localIP().toString();
            json["wifi_hostname"] = WiFi.getHostname();
            serializeJson(json, *response);
            request->send(response); });

        // config file download
        server.serveStatic("/config/", FFat, "/config/")
            .setCacheControl("no-store")
            .setDefaultFile("index.html");

        // gather information about connection status
        server.on("/status/filesystem", HTTP_GET, [](AsyncWebServerRequest *request)
                  { return false; }); // TODO: maybe add or not...
    }

    bool cb_api_cmd_save_configuration()
    {
        // send event to inform other modules to action
        Event ev("*/saveconfiguration");
        EventManager::instance().publish(ev);

        return true;
    }

    bool cb_api_cmd_load_configuration()
    {
        // send event to inform other modules to action
        Event ev("*/loadconfiguration");
        EventManager::instance().publish(ev);

        return true;
    }

    bool cb_api_cmd_tare()
    {
        // send event to inform other modules to action
        Event ev("Loadcell/tare");
        EventManager::instance().publish(ev);

        return true;
    }

    bool cb_api_cmd_calibrateknownreference(String knownreference_value)
    {
        // send event to inform other modules to action
        DataEvent ev("Loadcell/calibrateToKnownValue", knownreference_value);
        EventManager::instance().publish(ev);

        return true;
    }

    bool cb_api_config()
    {
        return false;
    }

    // simple commands
    void
    route_api_cmd_init()
    {
        // Send a POST request to <IP>/post with a form field message set to <message>
        server.on("/api/cmd/tare", HTTP_POST, [](AsyncWebServerRequest *request)
                  {
                    Serial.print("webserver tare triggered");

                    if (cb_api_cmd_tare())
                        request->send(200, "text/plain", "OK");

                    request->send(400, "text/plain", "request error"); });

        // Send a POST request to <IP>/post with a form field message set to <message>
        server.on("/api/cmd/loadconfiguration", HTTP_POST, [](AsyncWebServerRequest *request)
                  {
                    Serial.print("webserver loadconfiguration triggered");

                    if (cb_api_cmd_load_configuration())
                        request->send(200, "text/plain", "OK");

                    request->send(400, "text/plain", "request error"); });

        // Send a POST request to <IP>/post with a form field message set to <message>
        server.on("/api/cmd/saveconfiguration", HTTP_POST, [](AsyncWebServerRequest *request)
                  {
                    Serial.print("webserver saveconfiguration triggered");

                    if (cb_api_cmd_save_configuration())
                        request->send(200, "text/plain", "OK");

                    request->send(400, "text/plain", "request error"); });

        // Send a POST request to <IP>/post with a form field message set to <message>
        server.on("/api/cmd/calibrateknownreference", HTTP_POST, [](AsyncWebServerRequest *request)
                  {
                    Serial.print("webserver calibrateknownreference triggered");

                    //validation
                    if (!request->hasParam("knownValue", true))
                        request->send(400, "text/plain", "parameter knownValue missing");

                    //get request information for callback
                    String knownValue = request->getParam("knownValue", true)->value();


                    if (cb_api_cmd_calibrateknownreference(knownValue))
                        request->send(200, "text/plain", "OK");

                    request->send(400, "text/plain", "request error"); });
    }

    // api/config update requests
    void route_api_config_init()

    {
        // server.on("/command", HTTP_POST, [](AsyncWebServerRequest *request, JsonVariant json)
        //           {
        //             JsonObject jsonObj = json.as<JsonObject>();
        //         uint8_t command = jsonObj["C"];
        //         String response;
        //         switch(command){
        //                         case 0:
        //                             response = "{\"SCAN\":\"received_scan\"}";
        //                             request -> send(200, "application/json", response);
        //                         return;
        //                         case 1: /*...similar stuff */
        //                             return;
        //                         case 2: /*...*/
        //                             return;
        //     } });

        // AsyncCallbackJsonWebHandler *handler = new AsyncCallbackJsonWebHandler("/api/config", [](AsyncWebServerRequest *request, JsonVariant json)
        //                                                                        {
        //                                                                            JsonObject jsonObj = json.as<JsonObject>();
        //                                                                            Serial.println(jsonObj); });
        // server.addHandler(handler);
        server.on("/api/config/test", HTTP_ANY, [](AsyncWebServerRequest *request)
                  {
            Serial.print("webserver api/config triggered");
            int params = request->params();
            for (int i = 0; i < params; i++)
            {
                AsyncWebParameter *p = request->getParam(i);
                if (p->isFile())
                { // p->isPost() is also true
                    Serial.printf("FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
                }
                else if (p->isPost())
                {
                    Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
                }
                else
                {
                    Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
                }
            }
            request->send(200, "text/plain", "OK"); });

        server.on("/api/config/sensor", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                    Serial.print("webserver api/config/system triggered");

                    g_Config.sensor_config.sensitivity+=0.1;
                    Serial.print(g_Config.sensor_config.sensitivity); 
                    //g_Config.printsomevarsfortesting();

                    printFile(String(CONFIG_DIR) + g_Config.sensor_config._filename);
                    
                    g_Config.saveConfiguration(g_Config.sensor_config);

                    printDirectory(FFat.open("/"));

                    request->send(200, "text/plain", "OK"); });
    }

    void route_sse_init()
    {
        // SSE ......

        events.onConnect([](AsyncEventSourceClient *client)
                         {
            if(client->lastId()){
                Serial.printf("Client reconnected! Last message ID that it gat is: %u\n", client->lastId());
            }
            
            client->send("init event session",NULL, millis(), 1000); });

        server.addHandler(&events);
    }

    void route_notfound_init(AsyncWebServerRequest *request)
    {
        request->send(404, "text/plain", "Not found");
    }

    void routes_init()
    {
        route_webapp_init();

        route_status_init();

        route_api_cmd_init();

        route_api_config_init();

        route_sse_init();

        server.onNotFound(route_notfound_init);
    }

    void register_events()
    {
        EventManager::instance().subscribe([](Event *ev)
                                           {
            if (ev->is("Webservice/sendMessage"))
            {
                Serial.println("Webservice/sendMessage");
                Serial.println(((DataEvent *)ev)->data());

                invokeSendEvent("message", ((DataEvent *)ev)->data());
        } });
    }

    void initialize()
    {
        routes_init();

        register_events();

        server.begin();
    }

    void invokeSendEvent(String event, String value)
    {
        events.send(value.c_str(), event.c_str(), millis());
    }
}
