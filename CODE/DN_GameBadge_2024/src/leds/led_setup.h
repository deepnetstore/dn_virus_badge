#pragma once

/**
 * Ring Leds:
 *  Pin: 17
 *  - animate state of health/infection
 *  - pulse if near other beacons are infected or special beacons
 *  - blink multiple times quickly, if infected
 *
 * Background Leds:
 *  Pin: 18
 *  - animate ble scanning
 */

#include <Adafruit_NeoPixel.h>

#include "memory/load_game.h"

// On Board LED
#define LED_PIN 15

// RING LEDS
#define RING_LED_PIN 17 // 17 if orig...
#define RING_LED_NUM 12
#define RING_LED_MAX_BRITE 80
Adafruit_NeoPixel ring_strip = Adafruit_NeoPixel(RING_LED_NUM, RING_LED_PIN, NEO_GRB + NEO_KHZ400);

// GRID LEDS
#define GRID_LED_PIN 11
#define GRID_LED_NUM 76
#define GRID_LED_MAX_BRITE 8
#define GRID_FADE_UP_MULT 2
Adafruit_NeoPixel grid_strip = Adafruit_NeoPixel(GRID_LED_NUM, GRID_LED_PIN, NEO_GRB + NEO_KHZ400);

QueueHandle_t ring_led_queue;
QueueHandle_t grid_led_queue;

void update_led_brightness(int led_brightness)
{
    debug("Updating led brightness:");
    debugln(led_brightness);

    ring_strip.setBrightness((led_brightness * 20) + 20);
    ring_strip.show();

    grid_strip.setBrightness((led_brightness * 10));
    grid_strip.show();
}

enum LEDEffect
{
    IDLE,     // doing nothing...
    SOLID,    // only needs duration to stay on for
    FLASH,    // needs number of blinks and duration of half a cycle
    PULSE,    // fade in fade out over a set rate
    SPIN,     // rotate a color for duration of time
    RAINBOW,  // show rainbow effect for duration of time
    SCANNING, // Scanning animation .. like submarine radar
    FIGHT,    // fight back like a white blood cell
    HEAL,     // do healing ring led
    DAMAGE,   // do damage ring led
    SPECIAL   // TBD
};

enum gridDensity_e
{
    MIN_DENSITY,    // 10
    LOW_DENSITY,    // 8
    MEDIUM_DENSITY, // 6
    HIGH_DENSITY,   // 4
    MAX_DENSITY,    // 2
};

enum ledFadeType
{
    FADE_LEDS,
    NOFADE_LEDS,
};

typedef struct
{
    uint32_t color;
    LEDEffect effect;
    int duration;
    int count;
} ringLedMessage_t;

typedef struct
{
    bool is_infected;
    int health;
    bool run = false;
    bool update = false;
    bool tier1_passed = false;
} gridLedMessage_t;

#include "vg_colors.h"

#include "grid.h"
#include "led_effects.h"
#include "serialDebug.h"

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos)
{
    WheelPos = 255 - WheelPos;
    if (WheelPos < 85)
    {
        return ring_strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    if (WheelPos < 170)
    {
        WheelPos -= 85;
        return ring_strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    WheelPos -= 170;
    return ring_strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void rainbow(uint8_t wait)
{
    uint16_t i, j;

    for (j = 0; j < 256; j++)
    {
        for (i = 0; i < ring_strip.numPixels(); i++)
        {
            ring_strip.setPixelColor(i, Wheel((i + j) & 255));
        }
        ring_strip.show();
        vTaskDelay(wait / portTICK_PERIOD_MS);
    }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait)
{
    uint16_t i, j;

    for (j = 0; j < 256 * 5; j++)
    { // 5 cycles of all colors on wheel
        for (i = 0; i < ring_strip.numPixels(); i++)
        {
            ring_strip.setPixelColor(i, Wheel(((i * 256 / ring_strip.numPixels()) + j) & 255));
        }
        ring_strip.show();
        vTaskDelay(wait / portTICK_PERIOD_MS);
    }
}

// Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait)
{
    for (int j = 0; j < 10; j++)
    { // do 10 cycles of chasing
        for (int q = 0; q < 3; q++)
        {
            for (uint16_t i = 0; i < ring_strip.numPixels(); i = i + 3)
            {
                ring_strip.setPixelColor(i + q, c); // turn every third pixel on
            }
            ring_strip.show();

            vTaskDelay(wait / portTICK_PERIOD_MS);

            for (uint16_t i = 0; i < ring_strip.numPixels(); i = i + 3)
            {
                ring_strip.setPixelColor(i + q, 0); // turn every third pixel off
            }
        }
    }
}

// Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait)
{
    for (int j = 0; j < 255; j += 64)
    { // cycle all 256 colors in the wheel
        for (int q = 0; q < 3; q++)
        {
            for (uint16_t i = 0; i < ring_strip.numPixels(); i = i + 3)
            {
                ring_strip.setPixelColor(i + q, Wheel((i + j) % 255)); // turn every third pixel on
            }
            ring_strip.show();

            vTaskDelay(wait / portTICK_PERIOD_MS);

            for (uint16_t i = 0; i < ring_strip.numPixels(); i = i + 3)
            {
                ring_strip.setPixelColor(i + q, 0); // turn every third pixel off
            }
        }
    }
}

// radar scanning animation
void scanningRingLed(uint8_t wait, uint32_t colorChoice)
{
#define MODVAL 6
    for (int scanningRingLed_i = 0; scanningRingLed_i < MODVAL; scanningRingLed_i++)
    {
        ring_strip.setPixelColor(scanningRingLed_i, colorChoice);
        ring_strip.setPixelColor(scanningRingLed_i + MODVAL, colorChoice);
        ring_strip.show();

        vTaskDelay(pdMS_TO_TICKS(wait));

        ring_strip.setPixelColor(scanningRingLed_i, 0);
        ring_strip.setPixelColor(scanningRingLed_i + MODVAL, 0);
        ring_strip.show();
    }
    vTaskDelay(pdMS_TO_TICKS(wait * 2));
}

// setup
void setup_leds()
{
    debugln("Setting Up LEDs");

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    ring_led_queue = xQueueCreate(4, sizeof(ringLedMessage_t));
    grid_led_queue = xQueueCreate(4, sizeof(gridLedMessage_t));

    // SETUP
    ring_strip.begin();
    ring_strip.setBrightness(RING_LED_MAX_BRITE);
    ring_strip.clear();

    grid_strip.begin();
    grid_strip.setBrightness(GRID_LED_MAX_BRITE);
    grid_strip.clear();

    ring_strip.show(); // Initialize all pixels to 'off'
    grid_strip.show(); // Initialize all pixels to 'off'
}

void led_intro()
{
    // INTRO ANIMATION
    grid_strip.clear();
    for (int h = 0; h < GRID_LEDS_HIGH; h++)
    {
        for (int w = 0; w < GRID_LEDS_WIDE; w++)
        {
            next_grid_colors[h][w] = grid_colors[h][w];
            grid_strip.setPixelColor(grid_map[h][w], grid_colors[h][w]);
        }
    }
    grid_strip.show();

    delay(400);

    update_led_brightness(led_brightness);

    debugln("LED's READY");
}