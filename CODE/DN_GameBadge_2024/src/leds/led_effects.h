#pragma once

#include <Arduino.h>

#include "grid.h"

int grid_orientation = 0;
uint32_t next_grid_colors[GRID_LEDS_HIGH][GRID_LEDS_WIDE];

// take one step of the matrix
void do_the_matrix(uint32_t choice_color = GREEN, long probFactor = 3, bool fade_in = true)
{
    // update streams
    for (int h = 0; h < GRID_LEDS_HIGH; h++)
    {
        for (int w = 0; w < GRID_LEDS_WIDE; w++)
        {
            // device is inverted
            uint32_t current_color = grid_colors[h][w];

            // if current color is not black, move it
            if (current_color != (uint32_t)0)
            {
                // turn off the current grid point
                next_grid_colors[h][w] = (uint32_t)0;

                // check if at end of column of grid
                if ((h + 1) != GRID_LEDS_HIGH)
                {
                    // if not at end
                    // move current color to the next row down.
                    next_grid_colors[h + 1][w] = fade_in ? current_color : choice_color;
                }
            }
            // if at the top row, and no color is present
            else if (h == 0 && current_color == 0)
            {
                // start a new stream at random
                if (random(0, probFactor) == 0)
                {
                    next_grid_colors[h][w] = choice_color;
                }
            }
        } // w
    } // h

    // show streams
    for (int h = 0; h < GRID_LEDS_HIGH; h++)
    {
        for (int w = 0; w < GRID_LEDS_WIDE; w++)
        {
            grid_colors[h][w] = next_grid_colors[h][w];
            grid_strip.setPixelColor(grid_map[grid_orientation ? GRID_LEDS_HIGH - 1 - h : h][grid_orientation ? GRID_LEDS_WIDE - 1 - w : w], grid_colors[h][w]);
        }
    }

    grid_strip.show();
}