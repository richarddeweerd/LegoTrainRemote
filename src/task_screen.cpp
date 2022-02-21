#include "task_screen.h"
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include "TFT_Gui.h"

extern QueueHandle_t screenQueue;

TFT_eSPI tft = TFT_eSPI(); // Invoke library, pins defined in User_Setup.h
GUI_Screen screen = GUI_Screen(&tft);
GUI_Form bootpage = GUI_Form(&tft, TFT_GREEN);
GUI_Form page1 = GUI_Form(&tft, TFT_BLACK);

GUI_Taskbar taskbar = GUI_Taskbar(&tft, 0, 0, 0, 16, TFT_LIGHTGREY);
GUI_BatteryBar batterybar = GUI_BatteryBar(&tft, 104, 1, TFT_BLACK);
GUI_Form mainform = GUI_Form(&tft, 0, 16, 0, 0, TFT_BLUE);

GUI_Form train1form = GUI_Form(&tft, 2, 2, 61, 136, TFT_RED);
GUI_Form train2form = GUI_Form(&tft, 65, 2, 61, 136, TFT_RED);

void TaskScreen(void *pvParameters)
{
    // Task init
    tft.init();
    tft.setRotation(2);
    int currentpage = 0;

    GUI_Screen screen = GUI_Screen(&tft);
    screen.addChild(&bootpage);
    screen.addChild(&page1);
    page1.addChild(&taskbar);
    page1.addChild(&mainform);
    taskbar.addChild(&batterybar);
    mainform.addChild(&train1form);
    mainform.addChild(&train2form);

    page1.setVisible(false);
    screen.init();

    screen.draw();

    screenMessage rcv_msg;

    for (;;)
    {

        if (xQueueReceive(screenQueue, (void *)&rcv_msg, 0) == pdTRUE)
        {
            switch (rcv_msg.type)
            {
            case SCREEN_MSG_TYPE::PAGE:
                switch (rcv_msg.value)
                {
                case 0:
                    page1.setVisible(false);
                    bootpage.setVisible(true);
                    break;
                case 10:
                    page1.setVisible(true);
                    bootpage.setVisible(false);
                    break;

                default:
                    break;
                }
                break;
            case SCREEN_MSG_TYPE::EXT_POWER:
                batterybar.setExternalPower(rcv_msg.value);
                break;
            case SCREEN_MSG_TYPE::BAT:
                batterybar.setBatLevel(rcv_msg.value);
                break;
            case SCREEN_MSG_TYPE::WIFI:
                break;
            case SCREEN_MSG_TYPE::RSSI:
                break;
            case SCREEN_MSG_TYPE::MSG:
                break;
            case SCREEN_MSG_TYPE::SERVER:
                break;
            }
        }
        delay(25);
    }
}
