/*
setup
    storage
    input
    state machine
    menu
    leds
    espnow
    ble
tasks
    read inputs --> update state machine
    get states -->
        - update display menu
        - update leds
        - send communications
    get communications -->
        - espnow messages to task queue for state machine
        - state machine handle messages from queue
*/

#define TASK_CORE 0 // which core to run all tasks on
#define USE_ESP_NOW
#define USE_MOTION_SENSE
bool following_the_white_rabbit = true; // idk lol

#include <Arduino.h>

#include <Wire.h>

#include "serialDebug.h"

uint num_msg_tx = 0;
uint num_msg_rx = 0;

#include "battery.h"
#include "display.h"
#include "leds/led_setup.h"
#include "memory/setup.h"
#include "motion_sense.h"
#include "mutex/setup.h"
#include "vg_gamestate.h"
#include "vg_input.h"

#include "deepsleep.h"

#define SDA 33
#define SCL 34

#ifdef USE_ESP_BLE
#include "vg_ble/vg_ble_setup.h"
#endif

#ifdef USE_ESP_NOW
#include "espnow_setup.h"
#endif

#ifdef USE_MOTION_SENSE
#include "motion_sense.h"
#endif

TaskHandle_t xGridLEDHandle = NULL;
TaskHandle_t xRingLEDHandle = NULL;
TaskHandle_t xUIHandle = NULL;
TaskHandle_t xMotionSensorHandle = NULL;
TaskHandle_t xStateMachineHandle = NULL;

void dn_game_setup()
{
    //
    // setup mutex and semaphores
    //
    xSemaphore_i2c = xSemaphoreCreateMutex();

    //
    // setup periferals
    //
    setup_spiffs();         // load internal storage data
    setup_display();        // oled i2c display
    setup_leds();           // ring and grid leds
#ifdef USE_ESP_NOW          //
    setup_espnow();         //
#endif                      //
#ifdef USE_ESP_BLE          //
    setup_ble();            // ble beacon and scanning
#endif                      //
    setup_motion_sensor();  // motion input LSM6DSR
    setup_user_interface(); // input from buttons/touch etc
    setup_ui_menu();        // prep ui menu

    debugf("game_stats_health: %d \n", game_stats_health);
    debugf("game_stats_score: %d \n", game_stats_score);
    debugf("player_id: %d \n", player_id);

    display_intro();
    led_intro();

    //
    // start all tasks -- this is where all the action resides.
    //
    xTaskCreatePinnedToCore(grid_led_task, "GRID LEDS Task", 10000, NULL, 2, &xGridLEDHandle, TASK_CORE);
    xTaskCreatePinnedToCore(ring_led_task, "RING LEDS Task", 10000, NULL, 2, &xRingLEDHandle, TASK_CORE);
    xTaskCreatePinnedToCore(user_interface_task, "User Interface", 10000, NULL, 2, &xUIHandle, TASK_CORE);
#ifdef USE_MOTION_SENSE
    if (motionSensorEnabled)
    {
        debugln("Motion Sensor Enabled, starting motion sensor task");
        xTaskCreatePinnedToCore(motion_sensor_task, "GRID LEDS Task", 10000, NULL, 1, &xMotionSensorHandle, TASK_CORE);
    }
#endif

    xTaskCreatePinnedToCore(statemachine_task, "State Machine Task", 10000, NULL, 1, &xStateMachineHandle, TASK_CORE);
}

void setup()
{
    debugBegin(115200); // TODO USE DEBUG TYPE
    wakeup_routine();
    if (get_battery_voltage() < (MIN_BAT_VOLTAGE + 0.05))
    {
        // too low battery level to run. stop trying.
        do_badge_deep_sleep();
    }

    // Print the wakeup reason for ESP32
    setCpuFrequencyMhz(CPU_FREQ); //
    randomSeed(analogRead(12));   // use a pin with no connections
    randomSeed(random());

    debugln("Starting Game setup"); //
    Wire.begin(SDA, SCL);           //
    dn_game_setup();                //

    vTaskDelay(pdMS_TO_TICKS(8));

    debugln("All task have been created \nGame On."); //
    digitalWrite(LED_PIN, LOW);                       //
}

void loop()
{
    vTaskDelay(pdMS_TO_TICKS(999999)); // using freertos tasks
}