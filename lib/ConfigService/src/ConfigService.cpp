#include <ConfigService.hpp>
#include <ArduinoJson.h>
#include <FFat.h>

///
namespace ConfigService
{

    const char *filename = "/config.json";
    Config config; // <- global configuration object

    ///
    void initialize()
    {
        Serial.println("\nConfigService init");
        if (!FFat.begin(false, ""))
        {
            Serial.println("An error has occurred while mounting FFat");
            return;
        }

        // Should load default config if run for the first time
        Serial.println(F("Loading configuration..."));
        loadConfiguration(filename, config);

        // Dump config file
        Serial.println(F("Print config file..."));
        printFile(filename);
    }

    void update_loop()
    {
        return;
    }

    // ########### INTERNAL METHODS

    // Loads the configuration from a file
    void loadConfiguration(const char *filename, Config &config)
    {
        // Open file for reading
        File file = FFat.open(filename, FILE_WRITE, true);

        // Allocate a temporary JsonDocument
        // Don't forget to change the capacity to match your requirements.
        // Use arduinojson.org/v6/assistant to compute the capacity.
        StaticJsonDocument<512> doc;

        // Deserialize the JSON document
        DeserializationError error = deserializeJson(doc, file);
        if (error)
            Serial.println(F("Failed to read file, using default configuration"));

        // Copy values from the JsonDocument to the Config
        config.port = doc["port"] | 2731;
        strlcpy(config.hostname,                 // <- destination
                doc["hostname"] | "example.com", // <- source
                sizeof(config.hostname));        // <- destination's capacity

        // Close the file (Curiously, File's destructor doesn't close the file)
        file.close();

        // save defaults for next time loading.
        if (error)
            saveConfiguration(filename, config);
    }

    // Saves the configuration to a file
    void saveConfiguration(const char *filename, const Config &config)
    {
        // Delete existing file, otherwise the configuration is appended to the file
        FFat.remove(filename);

        // Open file for writing
        File file = FFat.open(filename, FILE_WRITE);
        if (!file)
        {
            Serial.println(F("Failed to create file"));
            return;
        }

        // Allocate a temporary JsonDocument
        // Don't forget to change the capacity to match your requirements.
        // Use arduinojson.org/assistant to compute the capacity.
        StaticJsonDocument<256> doc;

        // Set the values in the document
        doc["hostname"] = config.hostname;
        doc["port"] = config.port;

        // Serialize JSON to file
        if (serializeJson(doc, file) == 0)
        {
            Serial.println(F("Failed to write to file"));
        }

        // Close the file
        file.close();
    }

    // Prints the content of a file to the Serial
    void printFile(const char *filename)
    {
        // Open file for reading
        File file = FFat.open(filename);
        if (!file)
        {
            Serial.println(F("Failed to read file"));
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
}
