#pragma once

#include <Arduino.h>

#include "espnow_setup.h"
#include "led_setup.h"

bool g_score_teir1 = false;

uint32_t get_grid_health_color(int health)
{
    int offset = 0;
    int LED_MAX = 60;
    int val = map(health, -100, 100, 0, LED_MAX - offset);
    val = min(val, LED_MAX);
    val = max(0, val);
#ifdef DN_POWER_USER
#define COLORIZED_VALUE 12
    return grid_strip.Color(LED_MAX - (val + offset), COLORIZED_VALUE, val + offset);
#else
    int c_mod = 0;
    if (g_score_teir1)
    {
        return grid_strip.Color(LED_MAX - (val + offset), c_mod, val + offset);
    }
    else
    {
        return grid_strip.Color(LED_MAX - (val + offset), val + offset, c_mod);
    }
#endif
}

//
//
// GRID LEDS TASK
//
//
void grid_led_task(void *pv)
{
    unsigned long matrix_density = 99L;
    uint32_t matrix_color = get_grid_health_color(game_stats_health);
    int matrix_run_ts = millis();
    bool stopped_grid = false;

    while (following_the_white_rabbit)
    {
        // the matrix
        gridLedMessage_t msg;
        if (xQueueReceive(grid_led_queue, &msg, 60) == pdPASS)
        {
            if (msg.tier1_passed)
            {
                g_score_teir1 = true;
            }

            matrix_color = get_grid_health_color(msg.health);
            if (msg.update)
            {
                if (msg.run)
                {
                    matrix_density = 4L;
                    matrix_run_ts = millis();
                    stopped_grid = false;
                }
            }
        }

        if (leds_turned_on)
        {
            do_the_matrix(matrix_color, matrix_density, true);
        }
        vTaskDelay(pdMS_TO_TICKS(120)); // set rate for matrix rain

        if (millis() - matrix_run_ts > 1500 && !stopped_grid)
        {
            matrix_density = matrix_density < ((9L * 14L) + 4L) ? matrix_density + 9 : matrix_density;
            stopped_grid = true;
        }
    }

    while (!following_the_white_rabbit)
    {
        vTaskDelay(10000000);
    }
}
