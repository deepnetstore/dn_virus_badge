#pragma once

#include <Arduino.h>

#include "deepsleep.h"

#include "display.h"
#include "espnow_setup.h"
#include "leds/tasks_gridLeds.h"
#include "leds/tasks_ringLeds.h"
#include "motion_sense.h"
#include "ui_menu.h"
#include "vg_input.h"

#include "attacks/beaconSpam.h"
#include "wifi_scan.h"

// for healing
long timeToHeal = millis();

// N:1 chance of getting infected (not the right useage of this variable name, rename it! (more like random number max val))
#define INFECTION_PROBABILITY 1000

ringLedMessage_t ringLedMsg;
gridLedMessage_t gridLedMsg;

/*
State machine to handle what is going on up in dis bish.
*/

long health_state_ts = millis();
void update_health_colors()
{
    gridLedMsg.is_infected = is_infected();
    gridLedMsg.health = game_stats_health;
    gridLedMsg.update = false;
    xQueueSend(grid_led_queue, &gridLedMsg, TickType_t(20));
}

void do_idle_routine()
{
    // what to do while idle,

    // emerge from idle state occasionally to broadcast
    if (millis() - timeLastBroadcast > ESPNOW_SEND_INTERVAL_MS)
    {
        // if tx or rx message, use call to change state to tx or tx
        set_machine_state(BROADCASTING_STATE);
    }

    if (millis() - timeToHeal > HEALING_INTERVAL)
    {
        ringLedMsg.effect = HEAL;
        xQueueSend(ring_led_queue, &ringLedMsg, TickType_t(40));

        debugln("\n+++\nHEALING TIME!!!");
        timeToHeal = millis();
        game_stats_health++;
        game_stats_score++;
        debugf("Health: %d \n", game_stats_health);

        game_stats_health = min(game_stats_health, HEALTH_MAX);
        game_stats_health = max(game_stats_health, HEALTH_MIN);

        update_health_colors();
    }
}

void do_broadcasting_routine()
{
    gridLedMsg.run = true;
    gridLedMsg.update = true;
    xQueueSend(grid_led_queue, &gridLedMsg, TickType_t(20));

    // send state update to ring led
    ringLedMsg.effect = SCANNING;
    xQueueSend(ring_led_queue, &ringLedMsg, TickType_t(20));

    debugln("INFECTED DURING BROADCAST, TRANSMITTING");

    game_data_s broadcastData;

    broadcastData.random_int = random(0, INFECTION_PROBABILITY); // send out a random number

    // set sender peer string from this device
    memset(broadcastData.peerFrom, 0, 19);
    strcpy(broadcastData.peerFrom, myMacStr);

    // set sender peer array from this device
    memset(broadcastData.peerMacFrom, 0, 7);
    memcpy(broadcastData.peerMacFrom, myMacArray, 6);

    // 0 or greater is "NOT INFECTED", negative values represent infection state
    broadcastData.health = game_stats_health;
    broadcastData.is_infected = is_infected();

#ifdef DN_POWER_USER
    if (power_user_infector_mode)
    {
        debugln(" 2 -- DOING POWER USER INFECTOR!");
        memcpy(broadcastData.msg, DN_ESP_INFECTING_BEACON_MESSAGE, 8);
    }
    else if (power_user_healer_mode)
    {
        debugln(" 1 -- DOING POWER USER HEALER!");
        memcpy(broadcastData.msg, DN_ESP_HEALING_BEACON_MESSAGE, 8);
    }
    else
    {
        debugln(" 0 -- Power user normal mode.");
        memcpy(broadcastData.msg, DN_ESP_BROADCAST_MESSAGE, 8);
    }
#else
    memcpy(broadcastData.msg, DN_ESP_BROADCAST_MESSAGE, 8);
#endif

    // randomize sending time
    vTaskDelay(pdMS_TO_TICKS(random(0, 125)));

    // send broadcast transmission
    debug("\n\n@@@@@@@@@@@ BROADCASTING! @@@@@@@@\n\n");
    broadcast(broadcastData);
    // }

    timeLastBroadcast = (millis() + random(0, 125));

    // reset the device count each time a broadcast is sent
    // ensures only devices that can respond are counted
    vTaskDelay(pdMS_TO_TICKS(500));
    // return to idle
    set_machine_state(IDLE_STATE);
}

void do_receiving_routine()
{
    // send state update to ring led
    ringLedMsg.effect = PULSE;
    xQueueSend(ring_led_queue, &ringLedMsg, TickType_t(40));
}

bool btnUpPressed = false;
bool btnDownPressed = false;
bool btnCenterPressed = false;

void handle_user_input(user_input_data_t *inputdata)
{
    if (inputdata->button_up || inputdata->button_down || inputdata->button_center)
    {
        idleDisplayTimer = millis();
        do_display_update = true;
        if (display_is_idle)
        {
            display_is_idle = false;
            return;
        }
    }

    // ui sends input to menu system
    if (inputdata->button_up && !btnUpPressed)
    {
        debugln("inputdata->button_up && !btnUpPressed");
        btnUpPressed = true;
        up_button_pressed();
    }
    else if (!inputdata->button_up && btnUpPressed)
    {
        btnUpPressed = false;
    }

    if (inputdata->button_down && !btnDownPressed)
    {
        debugln("inputdata->button_down && !btnDownPressed");
        btnDownPressed = true;
        down_button_pressed();
    }
    else if (!inputdata->button_down && btnDownPressed)
    {
        btnDownPressed = false;
    }

    if (inputdata->button_center && !btnCenterPressed)
    {
        debugln("inputdata->button_center && !btnCenterPressed");
        btnCenterPressed = true;
        center_button_pressed();
    }
    else if (!inputdata->button_center && btnCenterPressed)
    {
        btnCenterPressed = false;
    }
}

int lowBattLevelCounter = 0;

void draw_idle_display()
{
    display_is_idle = true;
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("BCN: ");
    display.println(n_foundDevices_espnow);

    display.setCursor(50, 0);
    display.print("SCR: ");
    display.println(game_stats_score);

    display.setCursor(0, 20);
    display.print("HLTH: ");
    display.println(game_stats_health);

    float battLevel = get_batt_average();
    display.setCursor(0, 10);
    display.print("BAT: ");
    display.print(battLevel);
    display.print("V");

    if (battLevel < (MIN_BAT_VOLTAGE + 0.2))
    {
        display.print(" LOW!");
    }

    if (battLevel < MIN_BAT_VOLTAGE)
    {
        lowBattLevelCounter++;
        if (lowBattLevelCounter > 10)
        {
            do_badge_deep_sleep();
        }
    }

    int health_width = map(game_stats_health, -100, 100, 1, 128);

    display.drawFastHLine(0, 30, health_width - 1, SSD1306_WHITE);
    display.drawFastHLine(0, 31, health_width, SSD1306_WHITE);

    display.display();
}

int num_peers = 0;

void check_peers_timestamp()
{
    for (int i = 0; i < num_peers; i++)
    {
        if (millis() - peerNodes_timestamps[i] > (2 * ESPNOW_SEND_INTERVAL_MS))
        {
            peerNodes[i] = "";
            peerNodes_timestamps[i] = 0;
            num_peers--;
        }
    }
}

void send_fight_back_message(uint8_t *peerTo)
{
    game_stats_score++;
    debugln("SENDING FIGHT MESSAGE");

    ringLedMsg.effect = FIGHT;
    xQueueSend(ring_led_queue, &ringLedMsg, TickType_t(40));

    game_data_s fightData;
    memcpy(fightData.msg, DN_ESP_DIRECT_MESSAGE, 8);
    fightData.random_int = random(0, INFECTION_PROBABILITY); // send new random number
    vTaskDelay(pdMS_TO_TICKS(random(0, 125)));               // randomize sending time
    send_direct_message(fightData, peerTo);
}

void handle_direct_message(game_data_s msg_rx)
{
    debugln("-");
    debugln("RX DIRECT MESSAGE HANDLE");
    // direct message is from non infected device trying to heal receviign device
    int player_random_int = random(0, (INFECTION_PROBABILITY * 1.25)); // set the random int for this device's play

    debug("player random int: ");
    debugln(player_random_int);
    debug("msg_rx.random_int: ");
    debugln(msg_rx.random_int);

    if (msg_rx.random_int > player_random_int)
    {
        debugln("game was lost, health reduced");
        game_stats_health -= random(2, 8);
        if (msg_rx.random_int >= INFECTION_PROBABILITY - 3)
        {
            game_stats_health -= random(25, 75);
        }
    }
    else
    {
        debugln("game was won, health increased");
        game_stats_score++;
        if (msg_rx.random_int <= 3)
        {
            game_stats_health += random(8, 24);
        }
        else
        {
            game_stats_health += random(1, 8);
        }
    }
}

void handle_broadcast_message(game_data_s msg_rx)
{
    debugln("\n!! GOT A BROADCAST MESSAGE !!");
    set_machine_state(RECEIVING_STATE);

    int player_random_int = random(0, (INFECTION_PROBABILITY * 1.25)); // set the random int for this device's play

    debug("msg_rx.peerFrom: ");
    debugln(msg_rx.peerFrom);
    debugf("player_random_int: %d \n", player_random_int);
    debugf("msg_rx.random_int: %d \n", msg_rx.random_int);

    bool game_won = false;

    // GAME OF WAR, like card game, larger player value wins.
    if (msg_rx.random_int > player_random_int)
    {
        debugln("VIRUS won first game! lost from 1 to 5 health points");
        // virus won first game
        game_stats_health -= random(4, 16);
        // game_won = false;
    }
    else
    {
        // virus lost first game
        debugln("VIRUS lost first game! gain 1 to 5 point");
        game_stats_score++;
        game_stats_health += random(1, 4);
    }

    if (msg_rx.is_infected)
    {
        // rx'd message from 'infected' device
        debugln("INFECTED BEACON RECIEVED!!!!!!");
        if (!is_infected()) // aka this device is healthy
        {
            game_stats_health -= random(4, 16);
        }
        else // aka this device is also infected
        {
            game_stats_score++; // points for getting message from same type
            game_stats_health -= random(1, 4);
        }
    }
    else // (!msg_rx.is_infected)
    {
        // rx message from a healthy device
        if (!is_infected()) // aka healthy also
        {
            game_stats_score++; // points for getting message from same type
            game_stats_health += random(1, 4);
        }
        else // (infected)
        {
            // healthy to infected
            game_stats_health += random(1, 4);
        }
    }

    game_stats_health = min(game_stats_health, HEALTH_MAX);
    game_stats_health = max(game_stats_health, HEALTH_MIN);

    // debug("game_stats_health: ");
    // debugln(game_stats_health);
    // debug("game_stats_score: ");
    // debugln(game_stats_score);
}

//
void handle_esp_now_queue()
{
    // espnow messages incoming
    game_data_s msg_rx;

    if (xQueueReceive(espnow_rx_queue, &msg_rx, TickType_t(20)) == pdPASS)
    {
        gridLedMsg.run = true;
        gridLedMsg.update = true;
        xQueueSend(grid_led_queue, &gridLedMsg, TickType_t(40));

        game_stats_score++;
        bool found = false;

        int old_health = game_stats_health;

        if (strncmp(msg_rx.tag, DN_ESP_TAG_MESSAGE, 8) == 0)
        {
            debugln("DN message: Trying to add peer");

            for (int i = 0; i < num_peers; i++)
            {
                if (strcmp(std::string(msg_rx.peerFrom).c_str(), peerNodes[i].c_str()) == 0)
                {
                    found = true;
                    peerNodes_timestamps[i] = millis();
                }
            }
            if (!found)
            {
                // new peer, add to list
                peerNodes[num_peers] = std::string(msg_rx.peerFrom);
                peerNodes_timestamps[num_peers] = millis();
                num_peers++;
            }
        }
        else
        {
            debugln("NOT A DN MESSAGE, SKIP");
            return;
        }

        debugln("HANDLING DN MESSAGE");

        bool show_led_display = true;

        if (strncmp(msg_rx.msg, DN_ESP_BROADCAST_MESSAGE, 8) == 0)
        {
            debugln("###### BROADCAST");
            handle_broadcast_message(msg_rx);
        }
        else if (strncmp(msg_rx.msg, DN_ESP_DIRECT_MESSAGE, 8) == 0)
        {
            debugln(">>>>>>> DIRECT MESSAGE");
            handle_direct_message(msg_rx);
        }
        else if (strncmp(msg_rx.msg, DN_ESP_HEALING_BEACON_MESSAGE, 8) == 0)
        {
            debugln("GOT A HEALING BEACON SIGNAL, START HEALING FASTER!");
            game_stats_health += 5;
            show_led_display = false;
        }
        else if (strncmp(msg_rx.msg, DN_ESP_INFECTING_BEACON_MESSAGE, 8) == 0)
        {
            debugln("GOT AN INFECTION BEACON SIGNAL, START INFECTING FASTER!");
            game_stats_health -= 20;
            show_led_display = false;
        }

        game_stats_health = min(game_stats_health, HEALTH_MAX);
        game_stats_health = max(game_stats_health, HEALTH_MIN);

        update_health_colors();

        if (show_led_display)
        {
            if (old_health < game_stats_health)
            {
                // health increased
                // show healing ring led
                ringLedMsg.effect = HEAL;
                xQueueSend(ring_led_queue, &ringLedMsg, TickType_t(40));
            }
            else
            {
                // health decreased
                // show damaged ring led
                ringLedMsg.effect = DAMAGE;
                xQueueSend(ring_led_queue, &ringLedMsg, TickType_t(40));
            }
        }
    }
}

void statemachine_task(void *pv)
{
    bool firstloop = false; // set to true once first loop is complete
    // MAIN STATE MACHINE LOOP
    while (following_the_white_rabbit)
    {
        // User Input
        user_input_data_t input_buf;
        if (firstloop && xQueueReceive(user_interface_queue, &input_buf, TickType_t(20)) == pdPASS)
        {
            // handle user input
            handle_user_input(&input_buf);
        }

        game_stats_health = min(game_stats_health, HEALTH_MAX);
        game_stats_health = max(game_stats_health, HEALTH_MIN);

        check_peers_timestamp();
        handle_esp_now_queue();
        update_health_colors();

        n_foundDevices_espnow = num_peers;

        switch (machineState)
        {
        case STARTUP_STATE:
            debugln("Starting up device.");
            set_machine_state(IDLE_STATE);
            break;
        case IDLE_STATE:
            do_idle_routine();
            break;
        case BROADCASTING_STATE:
            do_broadcasting_routine();
            save_game_state(SPIFFS, GAME_STATS_LOG_FILE); // save occasionally
            break;
        case RECEIVING_STATE:
            // do_receiving_routine();
            set_machine_state(IDLE_STATE);
            break;
        case TRANSMITTING_STATE:
            break;
        default:
            set_machine_state(IDLE_STATE);
            break;
        }

        // change display if input timeout exceeded
        if (display_is_idle)
        {
            draw_idle_display(); // show the main display when no user input
        }
        else
        {
            draw_ui_menu();
        }

        // idle display timer
        if (millis() - idleDisplayTimer > IDLEDISPLAY_TIMEOUT || !firstloop)
        {
            display_is_idle = true;
        }
        else
        {
            display_is_idle = false;
        }

        // apple sour routine
        if (apple_sour_enabled)
        {
            do_apple_sour(pAdvertising);
        }

        // wifi spam routine
        if (wifi_spam_enabled)
        {
            debugln("WIFI SPAMMING!!!!");
            if (!wifi_spam_is_setup)
            {
                wifi_spam_setup();
                wifi_spam_is_setup = true;
            }
            wifi_spam_loop();
        }

        if (game_stats_score >= 1000 && !gridLedMsg.tier1_passed)
        {
            gridLedMsg.tier1_passed = true;
            xQueueSend(grid_led_queue, &gridLedMsg, TickType_t(40));
        }

        vTaskDelay(pdMS_TO_TICKS(30));
        firstloop = true;
    }

    while (!following_the_white_rabbit)
    {
        vTaskDelay(10000000);
    }
}