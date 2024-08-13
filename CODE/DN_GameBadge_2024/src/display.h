#pragma once

#include <Arduino.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <WiFi.h>

#include "serialDebug.h"

#include "dn_icon.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

bool display_enabled = false;

// views the display will show.
typedef enum
{
    WINDOW_IDLE,           // when nothing is happening... untouched forgotten...
    WINDOW_SPLASH,         // when device is turned on
    WINDOW_SCANNING,       // when BLE Scan is happening
    WINDOW_DISPLAY_SCORES, // typical view showing player's score and health state etc.
    WINDOW_CREDITS,        // maybe will get to this......
    WINDOW_CURRENT,        // update the current window, if label is not found: message is ignored
} WindowStates_e;

// parts of the window to apply updates to
typedef enum
{
    HEALTH,
    PLAYER_SCORE,
    BEACONS,
    BEACONS_BEST_RSSI,
    ACTION_STATE,
    CREDITS,
    OTHER
} WindowLabel_e;

enum disp_cmd
{
    UPDATE_DISP,
    ROTATION_DISP,
    INVERT_DISP,
};

// packet for display queue messages
typedef struct
{
    uint8_t window;
    uint8_t dataLabel;
    disp_cmd cmd;
    int value;
} displayMessage_t;

//
// setup queue and globals
QueueHandle_t displayQueue;

int currentWindow = WINDOW_IDLE;
int dataValues[3] = {0, 100, 0}; // Example data (points, health, beacons)

//
// SETUP DISPLAY
void setup_display()
{
    debugln("SETUP DISPLAY");

    // Initialize display (replace with your library initialization)
    displayQueue = xQueueCreate(10, sizeof(displayMessage_t)); // Create message queue with size 10

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        debugln(F("SSD1306 allocation failed"));
        display_enabled = false;
    }
    else
    {
        display_enabled = true;

        display.clearDisplay();
        display.display();

        display.setRotation(2);

        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);

        display.clearDisplay();
    }
}

void display_intro()
{
    for (int i = -34; i < (4 + 24); i++)
    {
        display.fillRect(i, 1, 32, 18, 0);
        display.drawBitmap(i, 2, dn_logo, 32, 16, 1);
        delay(2);
        display.display();
    }
    display.fillRect(4 + 24, 1, 32, 18, 0);
    display.drawBitmap(4 + 24, 2, dn_logo, 32, 16, 1);
    display.display();

    display.setCursor(42 + 20, 8);
#ifdef DN_POWER_USER
    display.print("POWER USER");
#else
    display.print("'VIRUS'");
#endif
    display.display();
    delay(400);

    for (int i = 2; i < (32 + 18); i++)
    {
        display.fillRect(4 + 24, i - 1, 32, 18, 0);
        display.drawBitmap(4 + 24, i, dn_logo, 32, 16, 1);
        display.display();
    }
}
