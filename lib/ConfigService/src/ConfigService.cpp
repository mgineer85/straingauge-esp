#include <ConfigService.hpp>
#include <DataEvent.hpp>

using namespace esp32m;

namespace ConfigService
{

    // SystemConfig config; // <- global configuration object

    void initialize()
    {

        // commands
        EventManager::instance().subscribe([](Event *ev)
                                           {
            if (ev->is("*/XXXXX"))
            {
                
            } });

        Serial.println("\nConfigService init");

        // Should load default config if run for the first time
        // Serial.println(F("Loading configuration..."));
        // loadConfiguration(config);
    }

    void update_loop()
    {
        return;
    }

    // ########### INTERNAL METHODS

    // Loads the configuration from a file
    void loadConfiguration(BaseConfig &config)
    {
        Serial.println("reading config file");
        Serial.println(String(CONFIG_DIR) + config._filename);

        // Open file for reading
        File file = FFat.open(String(CONFIG_DIR) + config._filename, FILE_READ);

        // Allocate a temporary JsonDocument
        // Don't forget to change the capacity to match your requirements.
        // Use arduinojson.org/v6/assistant to compute the capacity.
        StaticJsonDocument<512> doc;

        // Deserialize the JSON document
        DeserializationError error = deserializeJson(doc, file);
        if (error)
            Serial.println(F("Deserialization failed, using default configuration"));

        config.setStruct(doc);

        // Close the file (Curiously, File's destructor doesn't close the file)
        file.close();

        // save defaults for next time loading.
        if (error)
            saveConfiguration(config);
    }

    // Saves the configuration to a file
    void saveConfiguration(const BaseConfig &config)
    {
        Serial.println("writing config file");
        Serial.println(CONFIG_DIR + config._filename);

        // Delete existing file, otherwise the configuration is appended to the file
        FFat.remove(String(CONFIG_DIR) + config._filename);

        // Open file for writing
        File file = FFat.open(String(CONFIG_DIR) + config._filename, FILE_WRITE, true);
        if (!file)
        {
            Serial.println(F("Failed to create file"));
            return;
        }
        StaticJsonDocument<512> doc;
        config.setDoc(doc);

        // Serialize JSON to file
        if (serializeJson(doc, file) == 0)
        {
            Serial.println(F("Failed to write to file"));
        }

        // Close the file
        file.close();
    }

    // Prints the content of a file to the Serial
    void printFile(const String filename)
    {
        // Open file for reading
        File file = FFat.open(String(CONFIG_DIR) + filename);
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

    void printDirectory(File dir, int numTabs)
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
}
