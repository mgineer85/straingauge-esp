#include <Webservice.hpp>

#include <System.hpp>    // -->g_System
#include <Fuelgauge.hpp> // -->g_Fuelgauge
#include <Loadcell.hpp>  // -->g_Loadcell

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
            json["wifi_sta_ssid"] = WiFi.SSID();
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

        g_System.cbSaveConfiguration();
        g_Loadcell.cbSaveConfiguration();

        return true;
    }

    bool cb_api_cmd_load_configuration()
    {
        // send event to inform other modules to action
        g_System.cbLoadConfiguration();
        g_Loadcell.cbLoadConfiguration();

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

    // simple commands
    void route_api_cmd_init()
    {

        server.on("/api/cmd/restart", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                      log_i("webserver api/cmd/restart triggered");
                      request->send(200, "text/plain", "OK");

                      ESP.restart(); });

        // Send a HTTP_GET request to <IP>/post with a form field message set to <message>
        server.on("/api/cmd/tare", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                    log_i("webserver tare triggered");

                    if (cb_api_cmd_tare())
                        request->send(200, "text/plain", "OK");

                    request->send(400, "text/plain", "request error"); });

        // Send a HTTP_GET request to <IP>/post with a form field message set to <message>
        server.on("/api/cmd/loadconfiguration", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                    log_i("webserver loadconfiguration triggered");

                    if (cb_api_cmd_load_configuration())
                        request->send(200, "text/plain", "OK");

                    request->send(400, "text/plain", "request error"); });

        // Send a HTTP_GET request to <IP>/post with a form field message set to <message>
        server.on("/api/cmd/saveconfiguration", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                    log_i("webserver saveconfiguration triggered");

                    if (cb_api_cmd_save_configuration())
                        request->send(200, "text/plain", "OK");

                    request->send(400, "text/plain", "request error"); });

        // Send a POST request to <IP>/post with a form field message set to <message>
        server.on("/api/cmd/calibrateknownreference", HTTP_POST, [](AsyncWebServerRequest *request)
                  {
                    log_i("webserver calibrateknownreference triggered");

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
        // GET config
        server.on("/api/config", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
            log_i("webserver /api/config triggered");

            AsyncResponseStream *response = request->beginResponseStream("application/json");

            DynamicJsonDocument response_json(3072);
            DynamicJsonDocument system = DynamicJsonDocument(1024);
            DynamicJsonDocument sensor = DynamicJsonDocument(1024);
            DynamicJsonDocument adc = DynamicJsonDocument(1024);

            g_System.system_config.toDoc(system);
            g_Loadcell.sensor_config.toDoc(sensor);
            g_Loadcell.adc_config.toDoc(adc);

            // https://arduino.stackexchange.com/a/94216
            response_json[F("system")] = system;
            response_json[F("sensor")] = sensor;
            response_json[F("adc")] = adc;

            serializeJson(response_json, *response);

            request->send(response); });

        // POST/PUT/PATCH config
        AsyncCallbackJsonWebHandler *handlerCfg = new AsyncCallbackJsonWebHandler("/api/config", [](AsyncWebServerRequest *request, JsonVariant json)
                                                                                  {
                                                                                    
                                                                                    g_System.system_config.fromWeb(json["system"]);
                                                                                    g_Loadcell.sensor_config.fromWeb(json["sensor"]);
                                                                                    g_Loadcell.adc_config.fromWeb(json["adc"]);
                                                                                    g_Loadcell.postConfigChange();

                                                                                    String response = "{\"status\":\"OK\"}";
                                                                                    request -> send(200, "application/json", response); });
        server.addHandler(handlerCfg);
    }

    void route_sse_init()
    {
        // SSE ......

        events.onConnect([](AsyncEventSourceClient *client)
                         {
            if(client->lastId()){
                log_i("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
            }
            
            client->send("init event session",NULL, millis(), 1000); });

        server.addHandler(&events);
    }

    void route_notfound_init(AsyncWebServerRequest *request)
    {
        // TODO: send json if request is asking for json.

        String response_type = "text/plain";
        String response_content = "Not found";

        if (request->hasHeader("content-type"))
        {
            AsyncWebHeader *h = request->getHeader("content-type");

            if (h->value().equalsIgnoreCase("application/json"))
            {
                response_type = "application/json";
                response_content = "{\"error\":\"not found\"}";
            }
        }
        request->send(404, response_type, response_content);
    }

    void routes_init()
    {
        route_status_init();

        route_api_cmd_init();

        route_api_config_init();

        route_sse_init();

        // last init webapp - if noting else catched, this is kind of catchall before 404
        route_webapp_init();

        server.onNotFound(route_notfound_init);
    }

    void register_events()
    {
        EventManager::instance().subscribe([](Event *ev)
                                           {
            if (ev->is("Webservice/sendMessage"))
            {
                log_i("Webservice/sendMessage");
                log_d("%s",((DataEvent *)ev)->data().c_str());

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
