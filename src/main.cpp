#include <Arduino.h>
#include "hardware_config.h"
#include <SPIFFS.h>
#include "WiFi.h"
#include "WifiManager.h"

#include "ConfigStore.h"
#include "Rotary.h"
#include "TFT_eSPI.h"
#include "Button2.h"
#include "Matrix.h"

#include "task_screen.h"
#include "task_powermgmt.h"

byte matrix_x_pins[] = {PIN_MATRIX_X0, PIN_MATRIX_X1, PIN_MATRIX_X2, PIN_MATRIX_X3};
byte matrix_y_pins[] = {PIN_MATRIX_Y0, PIN_MATRIX_Y1, PIN_MATRIX_Y2, PIN_MATRIX_Y3};

#define CLICKS_PER_STEP 4 // this number depends on your rotary encoder
#define MIN_POS -100
#define MAX_POS 100
#define START_POS 0
#define INCREMENT 1

TaskHandle_t Screen_Task;
TaskHandle_t PowerManagement_Task;

QueueHandle_t screenQueue;
QueueHandle_t powermgmtQueue;

WiFiManager wm; // global wm instance

WiFiManagerParameterCheckBox mqtt_enabled;  // global param ( for non blocking w params )
WiFiManagerParameter mqtt_host;             // global param ( for non blocking w params )
WiFiManagerParameter mqtt_port;             // global param ( for non blocking w params )
WiFiManagerParameterPassword mqtt_password; // global param ( for non blocking w params )
// WiFiManagerParameterSpacer spacer_1;          // global param ( for non blocking w params )

Rotary r_1;
Rotary r_2;
RotaryCounter c_1;
RotaryCounter c_2;

Button2 button;
MatrixButton buttons[16];
Matrix matrix(matrix_x_pins, 4, matrix_y_pins, 4, buttons);

ConfigStore cfg;

void IRAM_ATTR ISR_1()
{
    r_1.loop();
}

void IRAM_ATTR ISR_2()
{
    r_2.loop();
}

void rotate(RotaryCounter &r)
{
    Serial.println(r.getPosition());
    // changed = true;
}

void sleep()
{
    // esp_sleep_enable_ext1_wakeup(0, ESP_EXT1_WAKEUP_ALL_LOW);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0); // 1 = High, 0 = Low

    // Go to sleep now
    Serial.println("Going to sleep now");
    delay(1000);
    esp_deep_sleep_start();
}

void longpress(Button2 &btn)
{
    digitalWrite(PIN_TFT_POWER, HIGH);
    ledcWrite(TFT_BRIGHTNESS, 0);
    delay(1000);
    sleep();
}

void pressed(MatrixButton &btn)
{
    Serial.print("Button ");
    Serial.print(btn.getId());
    Serial.println(" pressed.");
}

void sendScreenMessage(SCREEN_MSG_TYPE msg_type, int16_t val)
{
    screenMessage msg;
    msg.type = msg_type;
    msg.value = val;
    xQueueSend(screenQueue, (void *)&msg, 10);
}

String getParam(String name)
{
    // read parameter from server, for customhmtl input
    String value;
    if (wm.server->hasArg(name))
    {
        value = wm.server->arg(name);
    }
    return value;
}

void saveParamCallback()
{
    printf("\nSaving...\n");
    mqttData dat;
    dat.mqtt_enable = mqtt_enabled.isChecked();
    dat.mqtt_host = getParam("mqtt_host").c_str();
    dat.mqtt_port = getParam("mqtt_port").c_str();
    dat.mqtt_password = getParam("mqtt_pwd").c_str();

    printf("Mqtt Enable: %s\n", getParam("mqtt_enabled").c_str());
    printf("Mqtt host: %s\n", getParam("mqtt_host").c_str());
    printf("Mqtt port: %s\n", getParam("mqtt_port").c_str());
    printf("Mqtt pwd: %s\n", getParam("mqtt_pwd").c_str());

    // printf("checkbox1_param: %s\n", getParam("checkbox1_param").c_str());
    // Serial.println(checkbox1_param.isChecked()); //Alternative option
    // printf("checkbox2_param: %s\n", getParam("checkbox2_param").c_str());
    // Serial.println(checkbox2_param.isChecked()); //Alternative option
    // printf("select_param: %s\n", getParam("select_param").c_str());
    // printf("checkbox_param2: %s\n", getParam("checkbox_param2").c_str());
    // printf("radio_param: %s\n", getParam("radio_param").c_str());
    cfg.setMqttData(dat);
    cfg.saveData();
    delay(500);
    ESP.restart();
}

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);
    Serial.println(F("Booting..."));

    for (int i = 0; i < 16; i++)
    {
        buttons[i].begin(i);
        buttons[i].setPressedHandler(pressed);
    }
    matrix.begin();
    while (!SPIFFS.begin(true))
    {
        Serial.println(F("Failed to initialize SD library"));
        delay(1000);
    }
    // SPIFFS.format();
    Serial.println(F("Loading configuration..."));
    cfg.begin();
    cfg.print();
    Serial.println(F("Initializing powermanagement..."));

    powermgmtQueue = xQueueCreate(10, sizeof(powerMessage));

    xTaskCreatePinnedToCore(   // Use xTaskCreate() in vanilla FreeRTOS
        TaskPower,             // Function to be called
        "Power",               // Name of task
        4096,                  // Stack size (bytes in ESP32, words in FreeRTOS)
        NULL,                  // Parameter to pass to function
        0,                     // Task priority (0 to configMAX_PRIORITIES - 1)
        &PowerManagement_Task, // Task handle
        0);

    Serial.println(F("Initializing screen..."));
    pinMode(PIN_TFT_POWER, OUTPUT);
    digitalWrite(PIN_TFT_POWER, LOW);

    // setup backlight
    ledcSetup(0, 5000, 10);
    ledcAttachPin(PIN_TFT_LED, TFT_BRIGHTNESS);
    ledcWrite(TFT_BRIGHTNESS, 1023);

    screenQueue = xQueueCreate(10, sizeof(screenMessage));

    xTaskCreatePinnedToCore( // Use xTaskCreate() in vanilla FreeRTOS
        TaskScreen,          // Function to be called
        "Screen",            // Name of task
        4096,                // Stack size (bytes in ESP32, words in FreeRTOS)
        NULL,                // Parameter to pass to function
        0,                   // Task priority (0 to configMAX_PRIORITIES - 1)
        &Screen_Task,        // Task handle
        0);

    new (&mqtt_enabled) WiFiManagerParameterCheckBox("mqtt_enabled", "Enable MQTT", cfg.getMqttData().mqtt_enable);
    new (&mqtt_host) WiFiManagerParameter("mqtt_host", "MQTT Host", cfg.getMqttData().mqtt_host.c_str(), 40);
    new (&mqtt_port) WiFiManagerParameter("mqtt_port", "MQTT Port", cfg.getMqttData().mqtt_port.c_str(), 8);
    new (&mqtt_password) WiFiManagerParameterPassword("mqtt_pwd", "Password", "", 40);

    wm.addParameter(&mqtt_enabled);
    wm.addParameter(&mqtt_host);
    wm.addParameter(&mqtt_port);
    wm.addParameter(&mqtt_password);
    wm.setSaveParamsCallback(saveParamCallback);

    std::vector<const char *> menu = {"wifi", "info", "param", "sep", "erase", "update", "restart", "exit"};
    wm.setMenu(menu);
    wm.setConfigPortalTimeout(30);   // auto close configportal after n seconds
    wm.setCaptivePortalEnable(true); // disable captive portal redirection
    wm.setAPClientCheck(true);       // avoid timeout if client connected to softap

    // wifi scan settings
    // wm.setRemoveDuplicateAPs(false); // do not remove duplicate ap names (true)
    // wm.setMinimumSignalQuality(20);  // set min RSSI (percentage) to show in scans, null = 8%
    // wm.setShowInfoErase(false);      // do not show erase button on info page
    // wm.setScanDispPerc(true);       // show RSSI as percentage not graph icons

    // wm.setBreakAfterConfig(true);   // always exit configportal even if wifi save fails

    bool connect_result;
    connect_result = wm.autoConnect("AutoConnectAP", "password"); // password protected ap

    if (!connect_result)
    {
        printf("Failed to connect or hit timeout\n");
        // ESP.restart();
    }
    else
    {
        // if you get here you have connected to the WiFi
        printf("connected...yeey :)\n");
        wm.startWebPortal();
    }

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        printf("Connecting to WiFi..\n");
    }
    printf("Connected to the WiFi network\n");
    button.begin(PIN_POWER);
    // button.setLongClickHandler(longpress);
    button.setLongClickTime(2000);
    button.setLongClickDetectedHandler(longpress);

    c_1.begin(MIN_POS, MAX_POS, CLICKS_PER_STEP, START_POS, false);
    c_2.begin(MIN_POS, MAX_POS, CLICKS_PER_STEP, START_POS, false);
    r_1.begin(PIN_R1_A, PIN_R1_B, 10, &c_1);
    r_2.begin(PIN_R2_A, PIN_R2_B, 10, &c_2);

    c_1.setChangedHandler(rotate);
    c_2.setChangedHandler(rotate);

    attachInterrupt(PIN_R1_A, ISR_1, CHANGE);
    attachInterrupt(PIN_R1_B, ISR_1, CHANGE);
    attachInterrupt(PIN_R2_A, ISR_2, CHANGE);
    attachInterrupt(PIN_R2_B, ISR_2, CHANGE);
    sendScreenMessage(SCREEN_MSG_TYPE::PAGE, 10);
}

void loop()
{
    powerMessage pwr_msg;
    if (xQueueReceive(powermgmtQueue, (void *)&pwr_msg, 0) == pdTRUE)
    {
        sendScreenMessage(SCREEN_MSG_TYPE::EXT_POWER, pwr_msg.external_power);
        sendScreenMessage(SCREEN_MSG_TYPE::BAT, pwr_msg.battery_level);
    }

    button.loop();
    matrix.loop();
    wm.process();
}