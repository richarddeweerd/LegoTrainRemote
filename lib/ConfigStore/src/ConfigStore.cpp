#include "ConfigStore.h"
#include <ArduinoJson.h>
#include <SPIFFS.h>

ConfigStore::ConfigStore(){};

void ConfigStore::begin()
{
    loadData();
}

void ConfigStore::print()
{
    File file = SPIFFS.open(CONFIG_FILE);
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

void ConfigStore::saveData()
{
    // Delete existing file, otherwise the configuration is appended to the file
    SPIFFS.remove(CONFIG_FILE);

    // Open file for writing
    File file = SPIFFS.open(CONFIG_FILE, FILE_WRITE);
    if (!file)
    {
        Serial.println(F("Failed to create file"));
        return;
    }

    // Allocate a temporary JsonDocument
    // Don't forget to change the capacity to match your requirements.
    // Use arduinojson.org/assistant to compute the capacity.
    StaticJsonDocument<512> doc;

    // Set the values in the document
    doc["version"] = CONFIG_VERSION;
    // doc["minBat"] = minBatLevel;
    // doc["maxBat"] = maxBatLevel;

    // Serialize JSON to file
    if (serializeJson(doc, file) == 0)
    {
        Serial.println(F("Failed to write to file"));
    }

    // Close the file
    file.close();
};

void ConfigStore::loadData(){};
