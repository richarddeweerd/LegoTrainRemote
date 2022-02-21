#include "task_powermgmt.h"
#include <SPIFFS.h>
#include "ArduinoJson.h"
#include "hardware_config.h"

#include <movingAvg.h>


extern QueueHandle_t powermgmtQueue;

void saveBatteryCalibration(batteryCalibrationData cal)
{
    // Delete existing file, otherwise the configuration is appended to the file
    SPIFFS.remove(CALIBRATION_FILE);

    // Open file for writing
    File file = SPIFFS.open(CALIBRATION_FILE, FILE_WRITE);
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
    doc["minBat"] = cal.minBatLevel;
    doc["maxBat"] = cal.maxBatLevel;

    // Serialize JSON to file
    if (serializeJson(doc, file) == 0)
    {
        Serial.println(F("Failed to write to file"));
    }

    // Close the file
    file.close();
}

batteryCalibrationData loadBatteryCalibration()
{
    File file = SPIFFS.open(CALIBRATION_FILE);

    batteryCalibrationData cal;

    // Allocate a temporary JsonDocument
    // Don't forget to change the capacity to match your requirements.
    // Use arduinojson.org/v6/assistant to compute the capacity.
    StaticJsonDocument<256> doc;

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, file);
    if (error)
        Serial.println(F("Failed to read file, using default configuration"));

    // Copy values from the JsonDocument to the Config
    cal.minBatLevel = doc["minBat"] | 1900;
    cal.maxBatLevel = doc["maxBat"] | 2100;

    // Close the file (Curiously, File's destructor doesn't close the file)
    file.close();
    saveBatteryCalibration(cal);
    return cal;
}

bool checkExternalPower()
{
    uint16_t external_volt = analogRead(PIN_USB_VOLTAGE);
    if (external_volt > 200)
    {
        return true;
    }
    return false;
}

batteryCalibrationData updateCalibration(uint16_t raw_bat_volt, batteryCalibrationData cal)
{

    if (raw_bat_volt <= (cal.minBatLevel - 25))
    {
        cal.minBatLevel = raw_bat_volt;
        saveBatteryCalibration(cal);
    }

    if (raw_bat_volt >= (cal.maxBatLevel + 25))
    {
        cal.maxBatLevel = raw_bat_volt;
        saveBatteryCalibration(cal);
    }
    return cal;
}

uint16_t getBatLevel(uint16_t raw_bat_volt, batteryCalibrationData cal)
{
    uint16_t level = map(raw_bat_volt, cal.minBatLevel, cal.maxBatLevel, 0, 100);
    if (level > 100)
    {
        return 100;
    }
    if (level < 0)
    {
        return 0;
    }
    return level;
}

void sendPowerMessage(bool ext_power, int16_t level)
{
    powerMessage msg;
    msg.external_power = ext_power;
    msg.battery_level = level;
    xQueueSend(powermgmtQueue, (void *)&msg, 10);
}

void TaskPower(void *pvParameters)
{
    // Task init
    batteryCalibrationData cal = loadBatteryCalibration();

    pinMode(PIN_BAT_VOLTAGE, INPUT);
    pinMode(PIN_USB_VOLTAGE, INPUT);

    movingAvg bat_avg(10);
    bat_avg.begin();

    bool ext_power = checkExternalPower();
    uint16_t raw_bat_volt = analogRead(PIN_BAT_VOLTAGE);
    uint16_t bat_level = bat_avg.reading(getBatLevel(raw_bat_volt, cal));
    sendPowerMessage(ext_power, bat_level);
    for (;;)
    {
        bool new_ext_power = checkExternalPower();
        uint16_t raw_bat_volt = analogRead(PIN_BAT_VOLTAGE);

        if (!new_ext_power)
        {
            // check calibration
            cal = updateCalibration(raw_bat_volt, cal);
        }
        uint16_t new_bat_level = bat_avg.reading(getBatLevel(raw_bat_volt, cal));
        if ((ext_power != new_ext_power) || (bat_level != new_bat_level))
        {
            ext_power = new_ext_power;
            bat_level = new_bat_level;
            sendPowerMessage(ext_power, bat_level);
        }
        delay(500);
    }
}
