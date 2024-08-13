#pragma once

/*

Read battery levels and send to display task.

*/

#include <Arduino.h>

#include "serialDebug.h"

#define BATT_LEVEL_PIN 14
#define MIN_BAT_VOLTAGE 3.00

void setup_battery_monitor()
{
    pinMode(BATT_LEVEL_PIN, INPUT);
}

float get_battery_voltage()
{
    uint16_t analogVal = analogRead(BATT_LEVEL_PIN);
    return ((analogVal / 2048.0) * 3.3) + 0.2;
}

float get_batt_average(int N = 16)
{
    float total;
    for (int i = 0; i < N; i++)
    {
        total += get_battery_voltage();
        vTaskDelay(pdMS_TO_TICKS(8));
    }
    return (float)(total / N);
}