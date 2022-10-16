#pragma once

#include <cstdint>

void initGPIO();
uint16_t readGPIO();
void reportGPIO(uint8_t myId, uint16_t gpio);
uint8_t getAddr();