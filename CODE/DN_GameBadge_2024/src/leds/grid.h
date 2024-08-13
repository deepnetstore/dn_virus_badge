#pragma once

#include <Arduino.h>

#include "vg_colors.h"

#define NOLED GRID_LED_NUM
#define GRID_LEDS_WIDE 14
#define GRID_LEDS_HIGH 22

int grid_map[GRID_LEDS_HIGH][GRID_LEDS_WIDE] = {
    //  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
    {NOLED, NOLED, 16, NOLED, 71, 66, 67, 68, 69, 70, NOLED, 65, NOLED, NOLED},                      // 0
    {NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, 56}, // 1
    {NOLED, NOLED, 17, 73, 72, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, 64, 57, 55},                // 2
    {NOLED, NOLED, 18, 74, NOLED, 75, NOLED, NOLED, NOLED, NOLED, NOLED, 63, 58, 54},                // 3
    {NOLED, NOLED, 19, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, 62, 59, 53},          // 4
    {0, NOLED, 20, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, 60, 52},           // 5
    {NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, 61, 51},    // 6
    {1, 15, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED},     // 7
    {2, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, 50},     // 8
    {NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, 49}, // 9
    {NOLED, -2, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, 48},    // 10
    {NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, 47}, // 11
    {3, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, 46},     // 12
    {NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, 45}, // 13
    {4, 14, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, 44},        // 14
    {5, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, 43},     // 15
    {6, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, 42},     // 16
    {7, 13, 21, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, 41, NOLED, NOLED},           // 17
    {8, 12, 22, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, 40, NOLED, NOLED},           // 18
    {9, 11, 23, 25, 26, NOLED, NOLED, NOLED, 39, NOLED, 38, 37, NOLED, NOLED},                       // 19
    {NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, NOLED, 36, NOLED, NOLED}, // 20
    {NOLED, 10, 24, 27, 28, 29, 30, 31, 32, 33, 34, 35, NOLED, NOLED},                               // 21
};

uint32_t grid_colors[GRID_LEDS_HIGH][GRID_LEDS_WIDE] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 0
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, WHITE, 0}, // 4
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};
