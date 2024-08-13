#pragma once

#include <Arduino.h>

#include "esp_sleep.h"

/*

Inputs:
    Capacitive touch pins:
        touch right = io10
        touch center = io12
        touch left = io13

        (read with `int val = touchRead(pin_num);`, compare against a threshold value.)

    Buttons:
        button down = io36
        button center = io37
        button up = io38

        (read with `int val = digitalRead(pin_num);`)

    Other:
        i2c
        ble
        etc...

*/

#define TOUCH_PIN_RIGHT GPIO_NUM_10
#define TOUCH_PIN_CENTER GPIO_NUM_12
#define TOUCH_PIN_LEFT GPIO_NUM_13

#define TOUCH_THRESHOLD_RIGHT 50000
#define TOUCH_THRESHOLD_CENTER 55000
#define TOUCH_THRESHOLD_LEFT 52000

#define BUTTON_PIN_DOWN GPIO_NUM_36
#define BUTTON_PIN_CENTER GPIO_NUM_37
#define BUTTON_PIN_UP GPIO_NUM_38

QueueHandle_t user_interface_queue;

struct user_input_data_t
{
    bool touch_right = (bool)0;
    bool touch_center = (bool)0;
    bool touch_left = (bool)0;
    bool button_up = (bool)0;
    bool button_center = (bool)0;
    bool button_down = (bool)0;
};

void setup_user_interface()
{
    // setup input queue for sending input message to state machine
    user_interface_queue = xQueueCreate(8, sizeof(user_input_data_t));

    pinMode(BUTTON_PIN_UP, INPUT_PULLUP);
    pinMode(BUTTON_PIN_CENTER, INPUT_PULLUP);
    pinMode(BUTTON_PIN_DOWN, INPUT_PULLUP);
}

void user_interface_task(void *pv)
{
    user_input_data_t inputData;
    user_input_data_t prev_inputData;

    display_is_idle = false;

    while (following_the_white_rabbit)
    {
        // get user inputs
        // buttons
        inputData.button_up = (bool)!digitalRead(BUTTON_PIN_UP);
        inputData.button_center = (bool)!digitalRead(BUTTON_PIN_CENTER);
        inputData.button_down = (bool)!digitalRead(BUTTON_PIN_DOWN);

        // pass inputs on to state machine to handle
        if (prev_inputData.button_up != inputData.button_up || prev_inputData.button_center != inputData.button_center || prev_inputData.button_down != inputData.button_down)
        {
            xQueueSend(user_interface_queue, &inputData, TickType_t(20));
        }
        memcpy(&prev_inputData, &inputData, sizeof(user_input_data_t));

        vTaskDelay(pdMS_TO_TICKS(40));
    }

    while (!following_the_white_rabbit)
    {
        vTaskDelay(10000000);
    }
}