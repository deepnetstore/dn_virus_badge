#pragma once

#include "memory/load_game.h"

void print_wakeup_reason()
{
    esp_sleep_wakeup_cause_t wakeup_reason;

    wakeup_reason = esp_sleep_get_wakeup_cause();

    switch (wakeup_reason)
    {
    case ESP_SLEEP_WAKEUP_EXT0:
        Serial.println("Wakeup caused by external signal using RTC_IO");
        break;
    case ESP_SLEEP_WAKEUP_EXT1:
        Serial.println("Wakeup caused by external signal using RTC_CNTL");
        break;
    case ESP_SLEEP_WAKEUP_TIMER:
        Serial.println("Wakeup caused by timer");
        break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
        Serial.println("Wakeup caused by touchpad");
        break;
    case ESP_SLEEP_WAKEUP_ULP:
        Serial.println("Wakeup caused by ULP program");
        break;
    default:
        Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
        break;
    }
}

void do_badge_deep_sleep()
{
    Serial.println("DEEP SLEEP about to happen");
    // SAVE THE FUCKING GAME!!!!!
    save_game_state(SPIFFS, GAME_STATS_LOG_FILE);
    // end all tasks
    following_the_white_rabbit = false;
    vTaskDelay(pdMS_TO_TICKS(250));
    // turn off the leds
    ring_strip.clear();
    ring_strip.show();
    grid_strip.clear();
    grid_strip.show();
    delay(150);
    display.clearDisplay();
    display.setCursor(8, 8);
    display.print("POWERING DOWN.");
    display.display();
    delay(1000);
    display.print(".");
    display.display();
    delay(1000);
    display.print(".");
    display.display();
    delay(1000);
    // turn off the display
    display.clearDisplay();
    display.display();
    // do deep sleep imediately.
    delay(250);
    Serial.println("DEEP SLEEP BEGINGIN");
    esp_deep_sleep_start();
}

void wakeup_routine()
{
    print_wakeup_reason();
    /*
        First we configure the wake up source
        We set our ESP32 to wake up for an external trigger.
        There are two types for ESP32, ext0 and ext1 .
        ext0 uses RTC_IO to wakeup thus requires RTC peripherals
        to be on while ext1 uses RTC Controller so doesnt need
        peripherals to be powered on.
        Note that using internal pullups/pulldowns also requires
        RTC peripherals to be turned on.
    */
}
