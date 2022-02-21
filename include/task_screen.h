#ifndef TASK_SCREEN_H_
#define TASK_SCREEN_H_

#include <Arduino.h>

enum SCREEN_MSG_TYPE
{
    PAGE = 0,
    EXT_POWER = 1,
    BAT = 2,
    WIFI = 3,
    RSSI = 4,
    MSG = 5,
    SERVER = 6,

};

struct screenMessage
{
    SCREEN_MSG_TYPE type;
    int16_t value;
};

void TaskScreen(void *pvParameters);

#endif
