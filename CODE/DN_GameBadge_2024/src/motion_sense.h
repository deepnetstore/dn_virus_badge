#pragma once

// ESP32s3-fn8 has 320kb of RAM! 😬

#include <Arduino.h>
#include <LSM6DSRSensor.h>

#include <list>
#include <numeric>

#include "deepsleep.h"
#include "leds/led_setup.h"
#include "serialDebug.h"

#define MINUTES_INACTIVE 5 // 60 minutes for production

#define DEV_I2C Wire
#define INT_1 26
#define INT_2 35

bool napping = false;

double gyro_avg_x;
double gyro_avg_y;
double gyro_avg_z;
std::list<int> gyro_history_x;
std::list<int> gyro_history_y;
std::list<int> gyro_history_z;

LSM6DSRSensor AccGyr(&DEV_I2C, LSM6DSR_I2C_ADD_L);

bool motionSensorEnabled = false;
unsigned long last_active_state = 0UL;

void setup_motion_sensor()
{
    debugln("SETUP MOTION SENSOR");

    // Force INT1 of LSM6DSR low in order to enable I2C
    pinMode(INT_1, OUTPUT);
    digitalWrite(INT_1, LOW);

    int ret = AccGyr.begin();

    if (ret == 0)
    {
        motionSensorEnabled = true;

        // set up "wake on significant motion detection"
        // EMB_FUNC_STATUS_MAINPAGE
        // AccGyr.Write_Reg(LSM6DSR_EMB_FUNC_EN_A, 0b00110000);
        // AccGyr.Write_Reg(LSM6DSR_EMB_FUNC_STATUS, 0b00110000);
        // AccGyr.Write_Reg(LSM6DSR_EMB_FUNC_INIT_A, 0b00110000);
        // AccGyr.Write_Reg(LSM6DSR_EMB_FUNC_STATUS_MAINPAGE, 0b00110000);
        // AccGyr.Write_Reg(LSM6DSR_CTRL3_C, 0b0010010);

        AccGyr.Set_X_ODR_With_Mode(10.0, LSM6DSR_ACC_LOW_POWER_NORMAL_MODE);
        AccGyr.Set_G_ODR_With_Mode(10.0, LSM6DSR_GYRO_LOW_POWER_NORMAL_MODE);

        AccGyr.Set_G_FS(LSM6DSR_ACC_SENSITIVITY_FS_2G);
        AccGyr.Set_X_FS(LSM6DSR_GYRO_SENSITIVITY_FS_250DPS);

        AccGyr.Enable_X();
        AccGyr.Enable_G();

        pinMode(INT_1, INPUT_PULLDOWN);
        pinMode(INT_2, INPUT_PULLDOWN);

        // AccGyr.Write_Reg(LSM6DSR_TAP_CFG2, 0b11001111);
        // AccGyr.Write_Reg(LSM6DSR_INT1_CTRL, 0b11000000);
        // AccGyr.Write_Reg(LSM6DSR_INT2_CTRL, 0b11000000);
        // AccGyr.Write_Reg(LSM6DSR_, 0b00000001);

        debugln("Motion Sensor setup complete. OK");
    }
    else
    {
        debugln("Error setting up Motion sensor... please check assembly");
    }
}

void motion_test_loop()
{
    // TODO: Take i2c Semiphore

    // Read accelerometer and gyroscope.
    int32_t accelerometer[3];
    int32_t gyroscope[3];
    AccGyr.Get_X_Axes(accelerometer);
    AccGyr.Get_G_Axes(gyroscope);

    // Output data.
    // debug("ax:");
    debug(accelerometer[0]);
    // debug(", ay:");
    debug(", ");
    debug(accelerometer[1]);
    // debug(", az:");
    debug(", ");
    debug(accelerometer[2]);
    // debug(", gx:");
    debug(", ");
    debug(gyroscope[0]);
    // debug(", gy:");
    debug(", ");
    debug(gyroscope[1]);
    // debug(", gz:");
    debug(", ");
    debug(gyroscope[2]);
    debugln(" ");
}

double update_list_data_average(uint32_t newData, std::list<int> &dataList, int size = 64)
{
    double avg;
    if (dataList.size() >= size)
    {
        int sum = std::accumulate(dataList.begin(), dataList.end(), 0);
        avg = sum / size;
        // if vector list is too large, remove first entry, add new to end of list.
        dataList.pop_front();
    }
    dataList.push_back(newData);
    return avg;
}

void motion_check_device_orientation()
{
    int GYRO_THRESH = 90000;

    // Read accelerometer and gyroscope.
    int32_t accelerometer[3];
    int32_t gyroscope[3];
    AccGyr.Get_X_Axes(accelerometer);
    AccGyr.Get_G_Axes(gyroscope);

    gyro_avg_x = update_list_data_average(gyroscope[0], gyro_history_x);
    gyro_avg_y = update_list_data_average(gyroscope[1], gyro_history_y);
    gyro_avg_z = update_list_data_average(gyroscope[2], gyro_history_z);

    if (abs(gyroscope[0] - gyro_avg_x) > GYRO_THRESH)
    {
        last_active_state = millis();
        napping = false;
    }

    if (abs(gyroscope[1] - gyro_avg_y) > GYRO_THRESH)
    {
        last_active_state = millis();
        napping = false;
    }

    if (abs(gyroscope[2] - gyro_avg_z) > GYRO_THRESH)
    {
        last_active_state = millis();
        napping = false;
    }

    if (accelerometer[1] > 800)
    {
        // device is up right
        display.setRotation(2);
        grid_orientation = 0;
    }

    if (accelerometer[1] < -800)
    {
        // device is inverted
        display.setRotation(0);
        grid_orientation = 1;
    }
}

void motion_sensor_task(void *pv)
{
    debugln("Begin Motion Sensor Task");
    while (following_the_white_rabbit)
    {
        if (!napping && leds_turned_on)
        {
            if (led_brightness == 0)
            {
                led_brightness = 1;
                update_led_brightness(led_brightness);
            }
        }

        if (millis() - last_active_state > (MINUTES_INACTIVE * 60 * 1000) && !napping)
        {
            napping = true;
            // dim the lights?
            led_brightness = 0;
            update_led_brightness(led_brightness);
        }
        motion_check_device_orientation();
        vTaskDelay(pdMS_TO_TICKS(125));
    }

    while (!following_the_white_rabbit)
    {
        vTaskDelay(10000000);
    }
}
