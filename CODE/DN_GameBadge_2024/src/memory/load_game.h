#pragma once

/**
 *
 * Load and save the game state information and other data to store
 *
 * - ble scans, store the mac and number of times scanned
 * -- (? maybe) if scanned mac has been seen more than X number of times, become immune to that mac
 * -- this could be assumed to be a friend.
 *
 * -- other option:
 * -- let friends constantly dual as this will at least provide some excitement
 * -- during the event just incase they never cross another beacon...
 */

#include <ArduinoJson.h>
#include <SPIFFS.h>

#include "serialDebug.h"

// file to save all game states into
#define GAME_STATS_LOG_FILE "/dn_game_stats.txt"

long game_stats_health = 0;
long game_stats_score = 0;
long player_id = 0;
bool game_is_setup = false;
long is_infect_timestamp = millis();

// system level for led brightness
int led_brightness = 1;
// store led on/off state
bool leds_turned_on = true;

// returns bool(game_stats_health < 0)
bool is_infected()
{
    return game_stats_health <= 0;
}

void load_game_save_state(fs::FS &fs, const char *filename);
void save_game_state(fs::FS &fs, const char *filename);

// LOAD THE GAME SAVE STATE
void load_game_save_state(fs::FS &fs, const char *filename)
{
    // Allocate a temporary JsonDocument
    JsonDocument doc;

    // check if file exists
    File file = fs.open(filename);
    if (!file || file.isDirectory())
    {
        debugln("- failed to open file for reading");
        return;
    }

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, file);
    if (error)
    {
        debugln(F("Failed to read file, using default configuration"));
    }

    game_is_setup = doc["game_is_setup"] | false;

    game_stats_health = doc["game_stats_health"];
    game_stats_score = doc["game_stats_score"];
    led_brightness = doc["led_brightness"] | 1;
    leds_turned_on = doc["leds_turned_on"] | true;

    randomSeed(analogRead(random(1, 5))); // use a pin with no connections

    if (game_is_setup)
    {
        debugln("Game is setup, use loaded values.");
        player_id = doc["player_id"] | random(111110, 999999);
    }
    else
    {
        debugln("Game has not been setup, generate values.");
        player_id = random(111110, 999999);
        game_is_setup = true;
    }
}

void save_game_state(fs::FS &fs, const char *filename)
{
    // file will be replaced
    fs.remove(filename);

    // Open file for writing
    File file = fs.open(filename, FILE_WRITE);
    if (!file)
    {
        debugln(F("Failed to create file"));
        return;
    }

    // Allocate a temporary JsonDocument
    JsonDocument doc;

    // Set the values in the document
    doc["game_is_setup"] = game_is_setup;
    doc["game_stats_health"] = game_stats_health;
    doc["game_stats_score"] = game_stats_score;
    doc["player_id"] = player_id;
    doc["led_brightness"] = led_brightness;
    doc["leds_turned_on"] = leds_turned_on;

    if (serializeJson(doc, file) == 0)
    {
        debugln(F("Failed to write to file"));
    }

    // Close the file
    file.close();
}

// Prints the content of a file to the Serial
void printFile(fs::FS &fs, const char *filename)
{
    // Open file for reading
    File file = fs.open(filename);
    if (!file)
    {
        debugln(F("Failed to read file"));
        return;
    }

    // Extract each characters by one by one
    while (file.available())
    {
        debug((char)file.read());
    }
    debugln();

    // Close the file
    file.close();
}