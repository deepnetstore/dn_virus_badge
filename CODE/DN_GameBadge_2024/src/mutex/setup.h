#pragma once

#include <Arduino.h>

/*

Location for all mutexes/semaphore

I2C:
Mutex to avoid any potential i2c concurency issues.
its doubtful this


*/

// mutex/semaphore for i2c devices
SemaphoreHandle_t xSemaphore_i2c;
