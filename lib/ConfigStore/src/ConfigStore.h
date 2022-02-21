#ifndef Config_h
#define Config_h

#include <Arduino.h>

#define CONFIG_VERSION 1
#define CONFIG_FILE "/cfg.json"

class ConfigStore
{

public:
    ConfigStore();
    void begin();
    void print();
    void loadData();
    void saveData();

private:
    uint16_t version;
};

#endif