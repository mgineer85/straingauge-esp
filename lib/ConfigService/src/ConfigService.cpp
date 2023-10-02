#include <ConfigService.hpp>

ConfigClass g_Config;

ConfigClass::ConfigClass()
{
    // on init construct with default variables
    StaticJsonDocument<512> empty_doc;
    system_config.setStruct(empty_doc);
    sensor_config.setStruct(empty_doc);

    loadConfiguration(system_config);
    loadConfiguration(sensor_config);
}

// Loads the configuration from a file
void ConfigClass::loadConfiguration(BaseConfig &config)
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
void ConfigClass::saveConfiguration(const BaseConfig &config)
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
