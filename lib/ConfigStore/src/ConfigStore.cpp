#include "ConfigStore.h"
#include <ArduinoJson.h>
#include <SPIFFS.h>

ConfigStore::ConfigStore(){};

void ConfigStore::begin()
{
    loadData();
    if (version == 0)
    {
        saveData();
    }
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
    JsonObject mqtt = doc.createNestedObject("mqtt");
    mqtt["enable"] = mqtt_data.mqtt_enable;
    mqtt["host"] = mqtt_data.mqtt_host;
    mqtt["port"] = mqtt_data.mqtt_port;
    mqtt["pwd"] = mqtt_data.mqtt_password;

    // doc["maxBat"] = maxBatLevel;

    // Serialize JSON to file
    if (serializeJson(doc, file) == 0)
    {
        Serial.println(F("Failed to write to file"));
    }

    // Close the file
    file.close();
};

void ConfigStore::loadData()
{
    File file = SPIFFS.open(CONFIG_FILE);

    // Allocate a temporary JsonDocument
    // Don't forget to change the capacity to match your requirements.
    // Use arduinojson.org/v6/assistant to compute the capacity.
    StaticJsonDocument<256> doc;

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, file);
    if (error)
        Serial.println(F("Failed to read file, using default configuration"));

    // Copy values from the JsonDocument to the Config
    version = doc["version"] | 0;
    mqtt_data.mqtt_enable = doc["mqtt"]["enable"] | false;
    mqtt_data.mqtt_host = doc["mqtt"]["host"] | "mqtt.server.com";
    mqtt_data.mqtt_port = doc["mqtt"]["port"] | "1883";
    mqtt_data.mqtt_password = doc["mqtt"]["pwd"] | "";

    // Close the file (Curiously, File's destructor doesn't close the file)
    file.close();
};

void ConfigStore::setMqttData(mqttData dat)
{
    mqtt_data.mqtt_enable = dat.mqtt_enable;
    mqtt_data.mqtt_host = dat.mqtt_host;
    mqtt_data.mqtt_port = dat.mqtt_port;
    if (dat.mqtt_password.length() > 0)
    {
        mqtt_data.mqtt_password = dat.mqtt_password;
    }
}