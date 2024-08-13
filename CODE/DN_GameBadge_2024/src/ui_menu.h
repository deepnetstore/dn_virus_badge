#pragma once

/*

for creating the UI menu system to show on oled

*/

#include <Arduino.h>

#include "display.h"
#include "memory/setup.h"

#include "attacks/applesour.h"
#include "leds/led_setup.h"

#ifdef DN_POWER_USER
bool power_user_infector_mode = false;
bool power_user_healer_mode = false;
#endif

#define IDLEDISPLAY_TIMEOUT 5000
long idleDisplayTimer = millis() - IDLEDISPLAY_TIMEOUT;
bool display_is_idle = false;

int menu_index = 0;
int menu_length = 3;
int peer_menu_save_slot_index = 0;
const int peer_menu_max_save_slots = 8;

// store a temp mac from user input to save as immunity peer
int peer_save_mac_addr_chars[12] = {0x6, 0x4, 0xe, 0x8, 0x3, 0x3, 0x0, 0x0, 0x0, 0x0, 0x0};
int peer_menu_mac_addr_index = 0; // max 13?

// Formats MAC Address
void formatMacAddress_peerList(int *macAddr, char *buffer, int maxLength)
{
    snprintf(buffer, maxLength, "%01x%01x:%01x%01x:%01x%01x:%01x%01x:%01x%01x:%01x%01x",
             macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5],
             macAddr[6], macAddr[7], macAddr[8], macAddr[9], macAddr[10], macAddr[11]);
    debug("buffer: ");
    debugln(buffer);
}

bool do_display_update = true;
bool do_mac_edit = false;

enum button_inputs
{
    BTN_NONE,
    BTN_UP,
    BTN_CENTER,
    BTN_DOWN
};

button_inputs last_input = BTN_NONE;

// set up the menu item text and function call back
struct MenuItem
{
    int length;
    std::function<void()> update;
    std::function<void()> draw;
};

bool apple_sour_enabled = false;
bool wifi_spam_enabled = false;
bool wifi_spam_is_setup = false;

MenuItem ui_main_menu;
MenuItem ui_settings_menu;
MenuItem ui_brightness_menu;
MenuItem ui_extras_menu;
MenuItem ui_mac_menu;
MenuItem ui_power_user_menu;

MenuItem *p_current_menu; // store current active menu

void up_button_pressed()
{
    debugln("up_button_pressed()");
    last_input = BTN_UP;
}

void center_button_pressed()
{
    debugln("center_button_pressed()");
    last_input = BTN_CENTER;
}

void down_button_pressed()
{
    debugln("down_button_pressed()");
    last_input = BTN_DOWN;
}

void setup_ui_menu()
{
    ui_main_menu = {
        2,     // LENGTH
        []() { // UPDATE
            debugln("MAIN MENU UPDATE");
            switch (last_input)
            {
            case BTN_UP:
                debugln("BUTTON UP FROM MAIN Menu");
                menu_index = (--menu_index + menu_length) % menu_length;
                break;
            case BTN_CENTER:
                debugln("BUTTON CENTER FROM MAIN Menu");
                switch (menu_index)
                {
                case 0:
                    debugln("choice 0: settings");
                    p_current_menu = &ui_settings_menu;
                    menu_length = p_current_menu->length;
                    menu_index = 0;
                    break;
                case 1:
                    debugln("choice 1: extras");
                    p_current_menu = &ui_extras_menu;
                    menu_length = p_current_menu->length;
                    menu_index = 0;
                    init_sour();
                    break;
                case 2:
                    debugln("choice 2: ?");
                    break;
                }
                break;
            case BTN_DOWN:
                debugln("BUTTON DOWN FROM MAIN Menu");
                menu_index = ++menu_index % menu_length;
                break;
            }
        },
        []() { // DRAW
            debugln("SETTINGS MENU DRAW");
            display.setCursor(0, 0);
            display.println("  SETTINGS"); // 0
            display.println("  EXTRAS");   // 1

            switch (menu_index)
            {
            case 0:
                display.setCursor(0, 0);
                display.print(">");
                break;
            case 1:
                display.setCursor(0, 8);
                display.print(">");
                break;
            }
        },
    };

    // settings menu items
    ui_settings_menu = {
        3,     // LENGTH
        []() { // UPDATE SETTINGS
            debugln("SETTINGS MENU UPDATE");
            switch (last_input)
            {
            case BTN_UP:
                debugln("BUTTON UP FROM MAIN Menu");
                menu_index = (--menu_index + menu_length) % menu_length;
                break;
            case BTN_CENTER:
                debugln("BUTTON CENTER FROM MAIN Menu");
                switch (menu_index)
                {
                case 0:
                    debugln("choice 0");
                    p_current_menu = &ui_main_menu;
                    menu_length = p_current_menu->length;
                    menu_index = 0;
                    break;
                case 1:
                    debugln("choice 1 - ");
                    p_current_menu = &ui_mac_menu;
                    menu_length = p_current_menu->length;
                    menu_index = 0;
                    break;
                case 2:
                    debugln("choice 3 - brightness");
                    p_current_menu = &ui_brightness_menu;
                    menu_length = p_current_menu->length;
                    menu_index = 0;
                    break;
                }
                break;
            case BTN_DOWN:
                debugln("BUTTON DOWN FROM MAIN Menu");
                menu_index = ++menu_index % menu_length;
                break;
            }
        },
        []() { // DRAW SETTINGS
            debugln("DRAW");
            display.setCursor(0, 0);
            display.println("  BACK");       // 0
            display.println("  SHOW MAC");   // 1
            display.println("  BRIGHTNESS"); // 3

            switch (menu_index)
            {
            case 0:
                display.setCursor(0, 0);
                display.print(">");
                break;
            case 1:
                display.setCursor(0, 8);
                display.print(">");
                break;
            case 2:
                display.setCursor(0, 16);
                display.print(">");
                break;
            case 3:
                display.setCursor(0, 24);
                display.print(">");
                break;
            }
        },
    };

    ui_brightness_menu = {
        3,     // length
        []() { // update
            switch (last_input)
            {
            case BTN_UP:
                debugln("BUTTON UP FROM MAIN Menu");
                menu_index = (--menu_index + menu_length) % menu_length;
                break;
            case BTN_CENTER:
                switch (menu_index)
                {
                case 0:
                    p_current_menu = &ui_settings_menu;
                    menu_length = p_current_menu->length;
                    menu_index = 0;
                    break;
                case 1:
                    // increase brightness
                    led_brightness = (led_brightness < 10) ? (led_brightness + 1) : 10;
                    leds_turned_on = (led_brightness == 0) ? false : true;
                    update_led_brightness(led_brightness);
                    menu_index = 1;
                    break;
                case 2:
                    // decrease brightness
                    led_brightness = (led_brightness > 0) ? (led_brightness - 1) : 0;
                    leds_turned_on = (led_brightness == 0) ? false : true;
                    update_led_brightness(led_brightness);
                    menu_index = 2;
                    break;
                }
                break;
            case BTN_DOWN:
                debugln("BUTTON DOWN FROM MAIN Menu");
                menu_index = ++menu_index % menu_length;
                break;
            }
        },
        []() { // draw
            display.setCursor(0, 0);
            display.print("BRIGHTNESS: ");
            display.println(led_brightness);
            display.println("  BACK");     // 0
            display.println("  Increase"); // 1
            display.println("  Decrease"); // 2

            switch (menu_index)
            {
            case 0:
                display.setCursor(0, 8);
                display.print(">");
                break;
            case 1:
                display.setCursor(0, 16);
                display.print(">");
                break;
            case 2:
                display.setCursor(0, 24);
                display.print(">");
                break;
            }
        },
    };

    // extras menu, like attacks
    ui_extras_menu = {
#ifdef DN_POWER_USER
        4,
#else
        3, // length
#endif
        []() { // update
            switch (last_input)
            {
            case BTN_UP:
                debugln("up button pressed from extras menu");
                menu_index = (--menu_index + menu_length) % menu_length;
                break;
            case BTN_CENTER:
                debugln("center button pressed from extras menu");
                switch (menu_index)
                {
                case 0:
                    debugln("back");
                    p_current_menu = &ui_main_menu;
                    menu_length = p_current_menu->length;
                    menu_index = 0;
                    break;
                case 1:
                    debugln("toggle apple sour attack");
                    apple_sour_enabled = !apple_sour_enabled;
                    break;
                case 2:
                    debugln("toggle wifi spam attack");
                    wifi_spam_enabled = !wifi_spam_enabled;
                    break;
                case 3:
                    debugln("POWER USER Mode Settings loading");
                    p_current_menu = &ui_power_user_menu;
                    menu_length = p_current_menu->length;
                    menu_index = 0;
                    break;
                default:
                    break;
                }
                break;
            case BTN_DOWN:
                debugln("down button pressed from extras menu");
                menu_index = ++menu_index % menu_length;
                break;
            default:
                break;
            }
        },     // end update
        []() { // draw
            display.setCursor(0, 0);
            display.println("  BACK");
            display.println("  APPLE BLE SPAM");
            display.println("  WIFI SPAM");
#ifdef DN_POWER_USER
            display.println("  POWER USER");
#endif

            if (apple_sour_enabled)
            {
                display.setCursor(0, 8);
                display.print(" X");
            }

            if (wifi_spam_enabled)
            {
                display.setCursor(0, 16);
                display.print(" X");
            }

            switch (menu_index)
            {
            case 0:
                display.setCursor(0, 0);
                display.print(">");
                break;
            case 1:
                display.setCursor(0, 8);
                display.print(">");
                break;
            case 2:
                display.setCursor(0, 16);
                display.print(">");
                break;
            case 3:
                display.setCursor(0, 24);
                display.print(">");
                break;
            }
        }, // end draw
    };

#ifdef DN_POWER_USER
    ui_power_user_menu = {
        4,     // length
        []() { // update lambda
            debugln("power user menu");
            switch (last_input)
            {
            case BTN_UP:
                debugln("up ");
                menu_index = (--menu_index + menu_length) % menu_length;
                break;
            case BTN_CENTER:
                debugln("center");
                switch (menu_index)
                {
                case 0:
                    debugln("back");
                    p_current_menu = &ui_extras_menu;
                    menu_length = p_current_menu->length;
                    menu_index = 0;
                    break;
                case 1:
                    debugln("do healer power user");
                    power_user_healer_mode = true;
                    power_user_infector_mode = false;
                    break;
                case 2:
                    debugln("do infector power user");
                    power_user_healer_mode = false;
                    power_user_infector_mode = true;
                    break;
                case 3:
                    debugln("turn off power user mode");
                    power_user_healer_mode = false;
                    power_user_infector_mode = false;
                    break;
                default:
                    break;
                }
                break;
            case BTN_DOWN:
                debugln("down");
                menu_index = ++menu_index % menu_length;
                break;
            }

        },
        []() { // draw lambda
            display.setCursor(0, 0);
            display.println("  BACK");
            display.println("  HEALER");
            display.println("  INFECTOR");
            display.println("  OFF");

            if (power_user_healer_mode)
            {
                display.setCursor(0, 8);
                display.println(" X");
            }

            if (power_user_infector_mode)
            {
                display.setCursor(0, 16);
                display.println(" X");
            }

            switch (menu_index)
            {
            case 0:
                display.setCursor(0, 0);
                display.print(">");
                break;
            case 1:
                display.setCursor(0, 8);
                display.print(">");
                break;
            case 2:
                display.setCursor(0, 16);
                display.print(">");
                break;
            case 3:
                display.setCursor(0, 24);
                display.print(">");
                break;
            }
        },
    }; // end of ui_power_user_menu
#endif

    // show mac address
    ui_mac_menu = {
        1,     // LENGTH
        []() { // UPDATE MAC MENU
            debugln("mac menu");
            switch (last_input)
            {
            case BTN_CENTER:
                debugln("close mac display");
                p_current_menu = &ui_settings_menu;
                menu_length = p_current_menu->length;
                menu_index = 0;
                break;
            }
        },
        []() { // DRAW MAC MENU
            debugln("mac menu");
            display.setCursor(0, 0);
            display.println("MAC:");
            display.print(WiFi.macAddress().c_str());
            display.setCursor(0, 24);
            display.print("> BACK");
        },
    };

    // pick main menu to display.
    p_current_menu = &ui_main_menu;
    menu_length = p_current_menu->length;
}

void draw_ui_menu()
{
    if (!do_display_update)
    {
        return;
    }

    if (!xSemaphoreTake(xSemaphore_i2c, (TickType_t)10) == pdTRUE)
    {
        return;
    }

    display.clearDisplay();
    p_current_menu->update();
    p_current_menu->draw();
    display.display();

    xSemaphoreGive(xSemaphore_i2c);
    do_display_update = false;
}
