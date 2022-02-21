#ifndef TASK_POWERMGMT_H_
#define TASK_POWERMGMT_H_

#include <Arduino.h>

#define CALIBRATION_FILE "/cal.json"

struct batteryCalibrationData
{
    int16_t minBatLevel;
    int16_t maxBatLevel;
};

struct powerMessage
{
    bool external_power;
    int16_t battery_level;
};

batteryCalibrationData loadBatteryCalibration();
void saveBatteryCalibration(batteryCalibrationData cal);

bool checkExternalPower();

batteryCalibrationData updateCalibration(uint16_t raw_bat_volt, batteryCalibrationData cal);

void sendPowerMessage(bool ext_power, int16_t level);
uint16_t getBatLevel(uint16_t raw_bat_volt, batteryCalibrationData cal);

void TaskPower(void *pvParameters);

#endif
