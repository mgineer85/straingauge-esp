#include <System.hpp>

#define I2C_BUS_FREQUENCY 400000U

SystemClass g_System;

SystemClass::SystemClass()
{
    // on init construct with default variables
}

void SystemClass::initialize()
{

    log_i("System init");

    this->initialize_i2c();

    this->initialize_filesystem();

    this->cbLoadConfiguration();

    this->initialize_wifi();
}

void SystemClass::initialize_i2c()
{
    // probably already started master i2c by display, but no problem to begin again...
    Wire.begin();

    // By default .begin() will set I2C SCL to Standard Speed mode of 100kHz
    Wire.setClock(I2C_BUS_FREQUENCY);
}
void SystemClass::initialize_filesystem()
{

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
}
void SystemClass::initialize_wifi()
{
    // setup WiFi before webservice
    Display::status_message("WiFi setup...");

    WiFi.hostname(system_config.hostname);

    if (WiFi.isConnected())
        WiFi.disconnect();

    if (system_config.wifi_ap_mode)
    {
        log_i("Setup accesspoint WiFi SSID '%s'", system_config.wifi_ap_ssid);

        IPAddress ip(192, 168, 22, 1);
        IPAddress gateway(192, 168, 22, 1);
        IPAddress subnet(255, 255, 255, 0);
        WiFi.softAPConfig(ip, gateway, subnet);

        if (system_config.wifi_ap_ssid.length() >= 4 && system_config.wifi_ap_password.length() >= 8)
            WiFi.softAP(system_config.wifi_ap_ssid, system_config.wifi_ap_password);
        else
            WiFi.softAP("sg-box-failsafe", "12345678"); // invalid config - go failsafe wifi

        log_i("IP-Address: %s", ip.toString().c_str());
    }
    else
    {
        log_i("Connect to WiFi '%s'", system_config.wifi_sta_ssid);

        WiFi.mode(WIFI_STA);
        WiFi.begin(system_config.wifi_sta_ssid, system_config.wifi_sta_password);
        if (WiFi.waitForConnectResult(10000UL) != WL_CONNECTED)
        {
            log_e("WiFi failed!");
            Display::status_message("WiFi failed! Halted.");
            while (true)
                ;
        }

        log_i("Got IP-Address from WiFi: %s", WiFi.localIP().toString().c_str());
    }

    log_i("Hostname: %s", WiFi.getHostname());

    Display::status_message("WiFi setup finished.");
}

void SystemClass::cbSaveConfiguration(void)
{
    system_config.saveConfiguration();
}
void SystemClass::cbLoadConfiguration(void)
{
    system_config.loadConfiguration();
    postConfigChange();
}
void SystemClass::postConfigChange(void)
{
    log_i("postConfigChange triggered");

    // do not reinit wifi, since if would disconnect the client - shall only be called on reset or save config initialize_wifi();
}

void SystemClass::update_loop()
{
}

void SystemClass::printFilesystemFiles()
{
    Serial.println("Files on FFat:");
    printDirectory(FFat.open("/"));
}

// Prints the content of a file to the Serial
void SystemClass::printFile(const String filename)
{
    // Open file for reading
    File file = FFat.open(filename);
    if (!file)
    {
        log_e("Failed to read file");
        return;
    }

    // Extract each characters by one by one
    while (file.available())
    {
        Serial.print((char)file.read());
    }
    Serial.println();

    // Close the file
    file.close();
}

void SystemClass::printDirectory(File dir, int numTabs)
{
    while (true)
    {

        File entry = dir.openNextFile();
        if (!entry)
        {
            // no more files
            break;
        }
        for (uint8_t i = 0; i < numTabs; i++)
        {
            Serial.print('\t');
        }
        Serial.print(entry.name());
        if (entry.isDirectory())
        {
            Serial.println("/");
            printDirectory(entry, numTabs + 1);
        }
        else
        {
            // files have sizes, directories do not
            Serial.print("\t\t");
            Serial.println(entry.size(), DEC);
        }
        entry.close();
    }
}