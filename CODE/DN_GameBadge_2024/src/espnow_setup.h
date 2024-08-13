/*
    ESP-NOW uses the CCMP method, which is described in IEEE Std. 802.11-2012, to protect the vendor-specific action frame.
    The Wi-Fi device maintains a Primary Master Key (PMK) and several Local Master Keys (LMK).
    The lengths of both PMK and LMK need to be 16 bytes.

    PMK is used to encrypt LMK with the AES-128 algorithm. If PMK is not set, a default PMK will be used.

    LMK of the paired device is used to encrypt the vendor-specific action frame with the CCMP method.
    The maximum number of different LMKs is six. If the LMK of the paired device is not set, the vendor-specific
    action frame will not be encrypted.

    Encrypting multicast (broadcast address) vendor-specific action frame is not supported.

    PMK needs to be the same for all devices in the network. LMK only needs to be the same between paired devices.
*/

#pragma once

#include <Arduino.h>

#include <WiFi.h>
#include <esp_mac.h> // For the MAC2STR and MACSTR macros
#include <esp_now.h>
#include <esp_wifi.h>

#include "serialDebug.h"

#include "leds/led_setup.h"
#include "leds/vg_colors.h"
#include "memory/load_game.h"

// Wi-Fi interface to be used by the ESP-NOW protocol
#define ESPNOW_WIFI_IFACE WIFI_IF_STA

// Channel to be used by the ESP-NOW protocol
#define WIFI_CHANNEL 0
#define ESPNOW_WIFI_CHANNEL 0

// Delay between sending messages
#define ESPNOW_SEND_INTERVAL_MS 1 * 60 * 1000L // 1 minute, 60 seconds in a minute, 1000 ms in a second
// #endif

#define HEALING_INTERVAL 2 * 60 * 1000L // 10 seconds? 60 seconds? go up 1 health point?

#define USE_ESPNOW_ENCRYPTION false

// bool do_espnow_broadcast = true;
long timeLastBroadcast = millis() - ESPNOW_SEND_INTERVAL_MS;
long timeToReduceFoundDeviceCount = 0;
esp_now_peer_info_t peerInfo;
uint8_t myMacArray[6];
char myMacStr[18];

#define HEALTH_MIN -100L
#define HEALTH_MAX 100L
#define HEALING_AMOUNT 4
#define MAX_HEALTH_POINTS_CHANGE 20L
#define SCORE_POINTS_FOR_WIN 2
#define HEALTH_POINTS_FOR_LOSS 4

// send mac address of rx message to response queue
QueueHandle_t espnow_rx_queue;

#define DN_ESP_TAG_MESSAGE "DNDNDNDN"
#define DN_ESP_DIRECT_MESSAGE "DNDNDMDM"
#define DN_ESP_CONFIRMATION_MESSAGE "DNDNCONF"
#define DN_ESP_HEALING_BEACON_MESSAGE "DNDNHEAL"
#define DN_ESP_INFECTING_BEACON_MESSAGE "DNINFECT"
#define DN_ESP_BROADCAST_MESSAGE "DNDNDNDN"
#define DN_ESP_PING_MESSAGE "DNDNPING"

int n_foundDevices_espnow = 0;

struct peerNode
{
    std::string mac_address;
    long ts;
};

std::string peerNodes[64];
long peerNodes_timestamps[64];

enum response_type
{
    BROADCAST_ESPNOW,
    ACK_ESPNOW,
    CONNECT_ESPNOW,
    DISCONNECT_ESPNOW,
    OTHER_ESPNOW,
};

enum gameResult // result is from perspective of receiver of the broadcast
{
    GAME_WON,
    GAME_LOST,
    GAME_DRAW
};

struct game_data_s
{                                           // -1 values are unused unless it is score or health.
    bool is_infected = false;               // store state of virus game badge
    char tag[9] = DN_ESP_TAG_MESSAGE;       // keep something constant, use to verify sender.. could be more secure... :(
    char msg[9] = DN_ESP_BROADCAST_MESSAGE; // message type; broadcast, direct, or confirmation.
    char peerFrom[19];                      // always mac of sending device; in recevied message, it is device message is from.
    char peerTo[19];                        // intended destination of the message, good to check to not use wrong data.
    uint8_t peerMacFrom[7];                 // mac array of above.
    uint8_t peerMacTo[7];                   // mac array of above.
    int random_int = -1;                    // used for game playing, like the card game war...
    int real_code = -1;                     // after guess, reveal answer before changing to new code
    int score;                              // just to see, maybe make a scanner...
    int health;                             // same thing, just to show data.
    unsigned long ts;                       // time stamp for the events happening.
    gameResult game_won;                    // to send back game win or loss confirmation along with score etc.
};

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

char broadcastAddressStr[19] = "FF:FF:FF:FF:FF:FF";
#define PEER_DEVICE broadcastAddress

// FUNCTIONS

// Formats MAC Address
void formatMacAddress(const uint8_t *macAddr, char *buffer, int maxLength)
{
    snprintf(buffer, maxLength, "%02X:%02X:%02X:%02X:%02X:%02X", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
}

int extract_mac_address(char *mac_string, uint8_t *mac_bytes)
{
    /**
        get byte array from mac address string (like from above function)

        Extracts a MAC address from a string and stores it in a uint8_t array.

        Args : mac_string : The string containing the MAC address in colon -
        separated format(e.g., "DC:04:25:0A:08:00").mac_bytes : A uint8_t array of length 6 to store the extracted MAC address bytes.

                                                                Returns : True if the extraction is successful,
        False otherwise(e.g., invalid MAC address format or exceeding array bounds).
    */
    int items_scanned = sscanf(mac_string, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                               &mac_bytes[0], &mac_bytes[1], &mac_bytes[2],
                               &mac_bytes[3], &mac_bytes[4], &mac_bytes[5]);

    return items_scanned == 6; // Check if all 6 bytes were successfully scanned
}

bool add_espnow_peer(uint8_t *macAddr, bool use_encryption = false)
{
    memset(&peerInfo, 0, sizeof(peerInfo));
    const esp_now_peer_info_t *peer = &peerInfo;
    memcpy(peerInfo.peer_addr, macAddr, 6);

    peerInfo.channel = 0;              // pick a channel
    peerInfo.encrypt = use_encryption; // if to use encryption

    // check if the peer exists
    bool exists = esp_now_is_peer_exist(peerInfo.peer_addr);
    if (exists)
    {
        // Slave already paired.
        debugln("Already Paired");
        return true;
    }
    else
    {
        esp_err_t addStatus = esp_now_add_peer(peer);
        if (addStatus == ESP_OK)
        {
            // Pair success
            debugln("Pair success");
            return true;
        }
        else
        {
            debugln("Pair failed");
            return false;
        }
    }
}

// Called when data is sent
void sentCallback(const uint8_t *macAddr, esp_now_send_status_t status)
{
    num_msg_tx++;
    char macStr[18];
    formatMacAddress(macAddr, macStr, 18);
    debug("Last Packet Sent to: ");
    debugln(macStr);
    debug("Last Packet Send Status: ");
    debugln(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// Called when data is received
void receivedCallback(const uint8_t *macAddr, const uint8_t *data, int dataLen)
{
    num_msg_rx++;
    // Only allow a maximum of 250 characters in the message + a null terminating byte
    uint8_t buffer[ESP_NOW_MAX_DATA_LEN + 1];
    int msgLen = min(ESP_NOW_MAX_DATA_LEN, dataLen);
    memcpy(buffer, (void *)data, msgLen);

    // Make sure we are null terminated
    buffer[msgLen] = 0;

    // Format the MAC address
    char macStr[19];
    formatMacAddress(macAddr, macStr, 18);

    // Send Debug log message to the serial port
    debugf("\n-\nReceived message from: %s \n", macStr);

    // store message receieve in game data struct
    game_data_s gameData;
    // copy data into memory location of game data struct
    memcpy(&gameData, (uint8_t *)buffer, sizeof(gameData));

    // Check if DN Broadcast message received
    if (strncmp(gameData.tag, DN_ESP_TAG_MESSAGE, 8) == 0)
    {
        debugln("SENDING TO QUEUE");
        // ship it to the task queue
        debugln("<> Message sent to Queue task.\n");
        n_foundDevices_espnow++;
        xQueueSend(espnow_rx_queue, &gameData, TickType_t(40));
    }
    else
    {
        debugln("Ignoring non DN message");
    }
}

void send_direct_message(game_data_s txData, uint8_t *peer)
{
    // Format the MAC address
    char macStr[18];
    formatMacAddress(peer, macStr, 18);
    debug("Adding peer to try sending direct message == mac: ");
    debugln(macStr);

    add_espnow_peer(peer, USE_ESPNOW_ENCRYPTION);

    vTaskDelay(pdMS_TO_TICKS(10));

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(peer, (uint8_t *)&txData, sizeof(txData));

    if (result == ESP_OK)
    {
        debugln("++++Sending confirmed");
    }
    else
    {
        debugln("----Sending error");
    }
}

// Emulates a broadcast
void broadcast(game_data_s broadcastData)
{
    // Broadcast a message to every device in range
    add_espnow_peer(broadcastAddress, false);

    // Send message
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&broadcastData, sizeof(broadcastData));

    // Print results to serial monitor
    if (result == ESP_OK)
    {
        debugln("Broadcast message success");
    }
    else if (result == ESP_ERR_ESPNOW_NOT_INIT)
    {
        debugln("ESP-NOW not Init.");
    }
    else if (result == ESP_ERR_ESPNOW_ARG)
    {
        debugln("Invalid Argument");
    }
    else if (result == ESP_ERR_ESPNOW_INTERNAL)
    {
        debugln("Internal Error");
    }
    else if (result == ESP_ERR_ESPNOW_NO_MEM)
    {
        debugln("ESP_ERR_ESPNOW_NO_MEM");
    }
    else if (result == ESP_ERR_ESPNOW_NOT_FOUND)
    {
        debugln("Peer not found.");
    }
    else
    {
        debugln("Unknown error");
    }
}

// ===== Sniffer function ===== //
void sniff(void *buf, wifi_promiscuous_pkt_type_t type)
{
    debugln("sniffing packets rx'd");

    wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;

    uint8_t data = pkt->payload[pkt->rx_ctrl.sig_len];

    if (pkt->rx_ctrl.sig_mode == 0xA0 || pkt->rx_ctrl.sig_mode == 0xC0)
    {
        debugln("MAYBE DEAUTH?!");
    }

    debug("data->rx_ctrl.channel: ");
    debug(pkt->rx_ctrl.channel);
    debug(" - ");
    debug("data->rx_ctrl.rssi:");
    debugln(pkt->rx_ctrl.rssi);

    switch (type)
    {
    case WIFI_PKT_MGMT:
        debugln("WIFI_PKT_MGMT packet Received");
        break;
    case WIFI_PKT_CTRL:
        debugln("WIFI_PKT_CTRL packet Received");
        break;
    case WIFI_PKT_DATA:
        debugln("WIFI_PKT_DATA packet Received");
        break;
    case WIFI_PKT_MISC:
        debugln("WIFI_PKT_MISC packet Received - payload is zero.");
        break;

    default:
        break;
    }
}

// setup
void setup_espnow()
{
    debugln("Setting up ESPNOW");
    // enable task queue to handle incoming espnow messages
    espnow_rx_queue = xQueueCreate(8, sizeof(game_data_s));

    // Set the device as a Station
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    // enable long range mode, see if we can push this (WiFi may not work at same time?!)
    esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR);

    // Initilize ESP-NOW
    if (esp_now_init() != ESP_OK)
    {
        debugln("Error initializing ESP-NOW");
        debugln("ESP-NOW Init Failed");
        Serial.flush();
        delay(1000);
        ESP.restart();
        return;
    }
    debugln("ESP-NOW Init OK");

    // Register the send callback
    debug("esp_now_register_send_cb(sentCallback) : ");
    debugln(esp_now_register_send_cb(sentCallback));

    // Register callback function
    debug("esp_now_register_recv_cb(receivedCallback) : ");
    debugln(esp_now_register_recv_cb(receivedCallback));

    debugln("ESP-NOW Setup complete.");

    // Print MAC address
    debug("MAC Address: ");
    debugln(WiFi.macAddress());

    // store my mac for sending in messages
    strcpy(myMacStr, (char *)WiFi.macAddress().c_str());
    if (extract_mac_address(myMacStr, myMacArray))
    {
        debugf("My MAC address in bytes: ");
        for (int i = 0; i < 6; i++)
        {
            debugf("0x%02x ", myMacArray[i]);
        }
        debugf("\n");
    }
    else
    {
        debugf("Error: Invalid MAC address format\n");
    }
}