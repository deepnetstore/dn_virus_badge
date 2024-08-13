#pragma once

#include <Arduino.h>
#include <WiFi.h>

bool wifi_is_scanning = true;
int number_of_networks_found = 0;
int number_of_open_networks_found = 0;

void do_wifi_scan()
{
    // WiFi.scanNetworks will return the number of networks found
    number_of_open_networks_found = 0;
    WiFi.scanNetworks(true, false, false, 200U, 0U);
    wifi_is_scanning = true;

    number_of_networks_found = WiFi.scanComplete();

    while (number_of_networks_found < 0)
    {
        number_of_networks_found = WiFi.scanComplete();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    wifi_is_scanning = false;
    number_of_networks_found = WiFi.scanComplete();

    Serial.println("scan done");
    if (number_of_networks_found == 0)
    {
        Serial.println("no networks found");
    }
    else
    {
        Serial.print(number_of_networks_found);
        Serial.println(" networks found \n");
        for (int i = 0; i < number_of_networks_found; ++i)
        {
            // Print SSID and RSSI for each network found
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " !! OPEN !!" : "*");
            delay(10);

            //             if (WiFi.encryptionType(i) == WIFI_AUTH_OPEN)
            //             {
            //                 debugln("THIS IS AN OPEN NETWORK!!!! AAAHHHH");
            // #define MAX_STRENGTH 50
            //                 number_of_open_networks_found++;
            //                 int strength = map(WiFi.RSSI(i), -95, -35, 0, MAX_STRENGTH);
            //                 strength = max(strength, 1);
            //                 strength = min(strength, MAX_STRENGTH);
            //                 debug("Strength of open network damage: ");
            //                 debugln(strength);
            //                 game_stats_health -= (50 * strength); // lose health, closer is stronger
            //             }
            //             else if (WiFi.encryptionType(i) == WIFI_AUTH_WEP)
            //             {
            //                 // gain 1 point
            //                 game_stats_score++;
            //             }
            //             else if (WiFi.encryptionType(i) == WIFI_AUTH_WPA_PSK)
            //             {
            //                 // gaoin 2 points
            //                 game_stats_score += 2;
            //             }
            //             else if (WiFi.encryptionType(i) == WIFI_AUTH_WPA_WPA2_PSK)
            //             {
            //                 // gaoin 4 points
            //                 game_stats_score += 2;
            //             }
            //             else if (WiFi.encryptionType(i) == WIFI_AUTH_ENTERPRISE)
            //             {
            //                 // gaoin 8 points
            //                 game_stats_score += 4;
            //             }
            //             else if (WiFi.encryptionType(i) == WIFI_AUTH_WPA3_PSK)
            //             {
            //                 // gaoin 16 points
            //                 game_stats_score += 4;
            //             }
            //             else if (WiFi.encryptionType(i) == WIFI_AUTH_WPA2_WPA3_PSK)
            //             {
            //                 // gaoin 16 points
            //                 game_stats_score += 4;
            //             }
            //             else if (WiFi.encryptionType(i) == WIFI_AUTH_WPA3_ENT_192)
            //             {
            //                 // gaoin 16 points
            //                 game_stats_score += 4;
            //             }
            //             else if (WiFi.encryptionType(i) == WIFI_AUTH_MAX)
            //             {
            //                 // gaoin 32 points
            //                 game_stats_score += 4;
            //             }
        }
    }
    Serial.println("");
}

// task will run once then be deleted
void wifi_scan_task(void *pv)
{
    bool _running = true;
    while (_running)
    {
        do_wifi_scan();
        vTaskDelay(pdMS_TO_TICKS(50));
        vTaskDelete(NULL);
    }
}