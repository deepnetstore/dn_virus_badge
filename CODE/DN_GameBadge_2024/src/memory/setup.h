#pragma once

#include <Arduino.h>

#include "SPIFFS.h"
#include "serialDebug.h"

#include "memory/load_game.h"

// This file should be compiled with 'Partition Scheme' (in Tools menu)
// set to 'Default with ffat' if you have a 4MB ESP32 dev module or
// set to '16M Fat' if you have a 16MB ESP32 dev module.

enum machine_states
{
    STARTUP_STATE,
    IDLE_STATE,
    BROADCASTING_STATE,
    RECEIVING_STATE,
    TRANSMITTING_STATE
};

machine_states machineState = STARTUP_STATE;
long infected_timesstamp = 0;

void set_machine_state(machine_states newState)
{
    machineState = newState;
}

// You only need to format FFat the first time you run a test
#define FORMAT_FFAT true

bool listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
    debugf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if (!root)
    {
        debugln("- failed to open directory");
        return false;
    }
    if (!root.isDirectory())
    {
        debugln(" - not a directory");
        return false;
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            debug("  DIR : ");
            debugln(file.name());
            if (levels)
            {
                listDir(fs, file.path(), levels - 1);
            }
        }
        else
        {
            debug("  FILE: ");
            debug(file.name());
            debug("\tSIZE: ");
            debug(file.size());
            debugln(" bytes ");
        }
        file = root.openNextFile();
    }
    return true;
}

int readFile(fs::FS &fs, const char *path, int *file_data)
{
    /*
    Read file data in and save into *file_data
    must pass in destination of this through char array
    */
    debugf("Reading file: %s\r\n", path);

    File file = fs.open(path);
    if (!file || file.isDirectory())
    {
        debugln("- failed to open file for reading");
        return 1;
    }

    debugln("- read from file:");
    int i = 0;
    while (file.available())
    {
        int data = file.read();
        Serial.write(data);
        file_data[i++] = data;
    }
    file.close();
    return 0;
}

void writeFile(fs::FS &fs, const char *path, const char *message)
{
    debugf("Writing file: %s\r\n", path);

    File file = fs.open(path, FILE_WRITE);
    if (!file)
    {
        debugln("- failed to open file for writing");
        return;
    }
    if (file.print(message))
    {
        debugln("- file written");
    }
    else
    {
        debugln("- write failed");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message)
{
    debugf("Appending to file: %s\r\n", path);

    File file = fs.open(path, FILE_APPEND);
    if (!file)
    {
        debugln("- failed to open file for appending");
        return;
    }
    if (file.print(message))
    {
        debugln("- message appended");
    }
    else
    {
        debugln("- append failed");
    }
    file.close();
}

void renameFile(fs::FS &fs, const char *path1, const char *path2)
{
    debugf("Renaming file %s to %s\r\n", path1, path2);
    if (fs.rename(path1, path2))
    {
        debugln("- file renamed");
    }
    else
    {
        debugln("- rename failed");
    }
}

void deleteFile(fs::FS &fs, const char *path)
{
    debugf("Deleting file: %s\r\n", path);
    if (fs.remove(path))
    {
        debugln("- file deleted");
    }
    else
    {
        debugln("- delete failed");
    }
}

void testFileIO(fs::FS &fs, const char *path)
{
    debugf("Testing file I/O with %s\r\n", path);

    static uint8_t buf[512];
    size_t len = 0;
    File file = fs.open(path, FILE_WRITE);
    if (!file)
    {
        debugln("- failed to open file for writing");
        return;
    }

    size_t i;
    debug("- writing");
    uint32_t start = millis();
    for (i = 0; i < 2048; i++)
    {
        if ((i & 0x001F) == 0x001F)
        {
            debug(".");
        }
        file.write(buf, 512);
    }
    debugln("");
    uint32_t end = millis() - start;
    debugf(" - %u bytes written in %lu ms\r\n", 2048 * 512, end);
    file.close();

    file = fs.open(path);
    start = millis();
    end = start;
    i = 0;
    if (file && !file.isDirectory())
    {
        len = file.size();
        size_t flen = len;
        start = millis();
        debug("- reading");
        while (len)
        {
            size_t toRead = len;
            if (toRead > 512)
            {
                toRead = 512;
            }
            file.read(buf, toRead);
            if ((i++ & 0x001F) == 0x001F)
            {
                debug(".");
            }
            len -= toRead;
        }
        debugln("");
        end = millis() - start;
        debugf("- %u bytes read in %lu ms\r\n", flen, end);
        file.close();
    }
    else
    {
        debugln("- failed to open file for reading");
    }
}

//
//
//
// SETUP
//
void setup_spiffs()
{
    debugln("Starting FFat Mount...");

    Serial.setDebugOutput(true);

    if (!SPIFFS.begin())
    {
        debugln("SPIFFS Mount Failed");
        return;
    }

    debugf("Total space: %10u\n", SPIFFS.totalBytes(), " bytes ");

    printFile(SPIFFS, GAME_STATS_LOG_FILE);
    load_game_save_state(SPIFFS, GAME_STATS_LOG_FILE);
    save_game_state(SPIFFS, GAME_STATS_LOG_FILE);
    printFile(SPIFFS, GAME_STATS_LOG_FILE);

    debugln("Ended file system setup.");
}