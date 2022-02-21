#include <Arduino.h>
#include "hardware_config.h"
#include <SPIFFS.h>
#include "WiFi.h"

#include "ConfigStore.h"
#include "Rotary.h"
#include "TFT_eSPI.h"
#include "Button2.h"

#include "task_screen.h"
#include "task_powermgmt.h"

#define CLICKS_PER_STEP 4 // this number depends on your rotary encoder
#define MIN_POS -100
#define MAX_POS 100
#define START_POS 0
#define INCREMENT 1

TaskHandle_t Screen_Task;
TaskHandle_t PowerManagement_Task;

QueueHandle_t screenQueue;
QueueHandle_t powermgmtQueue;

Rotary r_1;
Rotary r_2;
RotaryCounter c_1;
RotaryCounter c_2;

Button2 button;

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

void sendScreenMessage(SCREEN_MSG_TYPE msg_type, int16_t val)
{
  screenMessage msg;
  msg.type = msg_type;
  msg.value = val;
  xQueueSend(screenQueue, (void *)&msg, 10);
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println(F("Booting..."));

  while (!SPIFFS.begin(true))
  {
    Serial.println(F("Failed to initialize SD library"));
    delay(1000);
  }
  // SPIFFS.format();
  Serial.println(F("Loading configuration..."));

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

  button.begin(PIN_POWER);
  // button.setLongClickHandler(longpress);
  button.setLongClickTime(2000);
  button.setLongClickDetectedHandler(longpress);

  Serial.print("Connecting to Wifi");
  WiFi.begin();
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected to the WiFi network");

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
  delay(1000);
}