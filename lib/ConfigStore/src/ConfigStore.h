#ifndef Config_h
#define Config_h

#include <Arduino.h>

#define CONFIG_VERSION 1
#define CONFIG_FILE "/cfg.json"

struct mqttData
{
    bool mqtt_enable;
    String mqtt_host;
    String mqtt_port;
    String mqtt_password;
};

class ConfigStore
{

public:
    ConfigStore();
    void begin();
    void print();
    void loadData();
    void saveData();
    mqttData getMqttData() { return mqtt_data; }
    void setMqttData(mqttData dat);

private:
    uint16_t version;
    mqttData mqtt_data;
};

#endif