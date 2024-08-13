#pragma once

#include <Arduino.h>

#include "serialDebug.h"

#include "led_effects.h"
#include "led_setup.h"
#include "vg_colors.h"

#define RING_LED_UPDATE_TIME 1000

enum ringLedStates
{
    IDLE_state,
    SOLID_state,
    FLASH_state,
    SPIN_state,
    RAINBOW_state,
    SPECIAL_state,
    SCANNING_state,
    RECEIVING_state,
    FIGHTING_state,
    HEALING_state,
    DAMAGED_state,
};

// pulse for idle for ring LEDS only!
long lastPulseTimer = millis();
int pulse_index = 0;

#define MAX_LED_BRITE 10
void pulse_leds(long waitTicks = 35)
{
    uint8_t brite;
    for (brite = 0; brite < MAX_LED_BRITE; brite++)
    {
        // going up
        ring_strip.fill(ring_strip.Color(brite * 3, 0, brite * 10));
        ring_strip.show();
        vTaskDelay(waitTicks);
    }
    for (brite = 0; brite < MAX_LED_BRITE; brite++)
    {
        // going down
        ring_strip.fill(ring_strip.Color(((MAX_LED_BRITE - 1) - brite) * 3, 0, ((MAX_LED_BRITE - 1) - brite) * 10));
        ring_strip.show();
        vTaskDelay(waitTicks * 2);
    }
}

void pulse_leds_green(long waitTicks = 80)
{
    for (int i = 0; i < 7; i++)
    {
        ring_strip.setPixelColor(6 - i, GREEN);
        ring_strip.setPixelColor(6 + i, GREEN);
        ring_strip.show();
        vTaskDelay(pdMS_TO_TICKS(waitTicks));
        ring_strip.setPixelColor(6 - i, 0);
        ring_strip.setPixelColor(6 + i, 0);
        ring_strip.show();
    }
}

void pulse_leds_red(long waitTicks = 80)
{
    for (int i = 7; i > 0; i--)
    {
        ring_strip.setPixelColor(6 - i, RED);
        ring_strip.setPixelColor(6 + i, RED);
        ring_strip.show();
        vTaskDelay(pdMS_TO_TICKS(waitTicks));
        ring_strip.setPixelColor(6 - i, 0);
        ring_strip.setPixelColor(6 + i, 0);
        ring_strip.show();
    }
}

//
//
// RING LEDS TASK
//
//
void ring_led_task(void *pv)
{
    bool found_devices = false;
    ringLedMessage_t msg;
    ringLedStates ringState = IDLE_state;
    long lastLEDupdateTime = millis();
    uint16_t rainbow_i = 0;

    // CLEAR
    ring_strip.clear();
    ring_strip.show();

    while (following_the_white_rabbit)
    {
        // receive from ring led queue color setting
        if (xQueueReceive(ring_led_queue, &msg, 10) == pdPASS)
        {
            switch (msg.effect)
            {
            case RAINBOW:
                ringState = RAINBOW_state;
                break;
            case SCANNING:
                ringState = SCANNING_state;
                break;
            case PULSE:
                ringState = RECEIVING_state;
                pulse_index = 0;
                lastPulseTimer = millis();
                break;
            case FIGHT:
                ringState = FIGHTING_state;
                pulse_index = 0;
                lastPulseTimer = millis();
                break;
            case HEAL:
                ringState = HEALING_state;
                break;
            case DAMAGE:
                ringState = DAMAGED_state;
                break;
            default:
                ringState = IDLE_state;
                pulse_index = 0;
                lastPulseTimer = millis();
                break;
            }
            lastLEDupdateTime = millis();
        }

        if (millis() - lastLEDupdateTime > RING_LED_UPDATE_TIME && ringState != IDLE_state)
        {
            ring_strip.clear();
            ring_strip.show();
            ringState = IDLE_state;
        }

        switch (ringState)
        {
        case IDLE_state:
            break;
        case RAINBOW_state:
            theaterChaseRainbow(120);
            break;
        case SCANNING_state:
            scanningRingLed(120, BROADCAST_COLOR);
            break;
        case RECEIVING_state:
            // show pulse for each received message.
            pulse_leds();
            ringState = IDLE_state;
            break;
        case FIGHTING_state:
            // show pulse for each received message.
            pulse_leds();
            ringState = IDLE_state;
            break;
        case HEALING_state:
            pulse_leds_green();
            ringState = IDLE_state;
            break;
        case DAMAGED_state:
            pulse_leds_red();
            ringState = IDLE_state;
            break;
        default:
            break;
        }

        ring_strip.show();

        vTaskDelay(pdMS_TO_TICKS(20));
    }

    while (!following_the_white_rabbit)
    {
        vTaskDelay(10000000);
    }
}
