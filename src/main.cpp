#include <Arduino.h>
#include "Rotary.h"
#include "TFT_eSPI.h"

#define PIN_R1_A 2
#define PIN_R1_B 15
#define PIN_R2_A 16
#define PIN_R2_B 4
#define PIN_TFT_POWER 22
#define PIN_TFT_LED 17
#define PIN_BAT_VOLTAGE 35
#define PIN_USB_VOLTAGE 39

#define CLICKS_PER_STEP 4 // this number depends on your rotary encoder
#define MIN_POS -100
#define MAX_POS 100
#define START_POS 0
#define INCREMENT 1

Rotary r_1;
Rotary r_2;
RotaryCounter c_1;
RotaryCounter c_2;

TFT_eSPI tft = TFT_eSPI();

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

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Booting");
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
  pinMode(PIN_TFT_POWER, OUTPUT);
  pinMode(PIN_TFT_LED, OUTPUT);

  pinMode(PIN_BAT_VOLTAGE, INPUT);
  pinMode(PIN_USB_VOLTAGE, INPUT);
  digitalWrite(PIN_TFT_POWER, LOW);
  digitalWrite(PIN_TFT_LED, HIGH);
  tft.init();
  tft.setRotation(2);

  tft.fillScreen(TFT_BLUE);
}

void loop()
{
  // put your main code here, to run repeatedly:
  tft.fillScreen(TFT_BLUE);
  uint16_t bat_volt = analogRead(PIN_BAT_VOLTAGE);
  uint16_t usb_volt = analogRead(PIN_USB_VOLTAGE);

  tft.drawNumber(map(bat_volt, 0, 4095, 0, 7250), 10, 10);
  tft.drawNumber(map(usb_volt, 0, 4095, 0, 7250), 10, 30);
  delay(1000);
  // sleep();
}