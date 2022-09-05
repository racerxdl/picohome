#include "pinctrl.h"

#include <Arduino.h>

// Millis
#define DEBOUNCE_TIME 50

#define BIT(v, n) (v & (1 << n))
#define SETBIT(v, n) (v | (1 << n))
#define CLRBIT(v, n) (v & ~(1 << n))

uint32_t lastDebounceTime[16];
uint16_t lastValue = 0;
uint16_t debouncedValue = 0;

void initGPIO() {
    pinMode(25, OUTPUT);  // LED
    digitalWrite(25, LOW);
    for (int i = 0; i < 16; i++) {
        lastDebounceTime[i] = 0;
        pinMode(i, INPUT_PULLUP);
    }
    lastValue = 0xFFFF;
    debouncedValue = 0xFFFF;
}

uint16_t readGPIO() {
    uint16_t currentGPIO = gpio_get_all() & 0xFFFF;
    for (int i = 0; i < 16; i++) {
        uint16_t lastVal = BIT(lastValue, i);
        uint16_t currVal = BIT(currentGPIO, i);
        if (lastVal != currVal) {
            lastDebounceTime[i] = millis();
        }

        if (millis() - lastDebounceTime[i] > DEBOUNCE_TIME) {
            if (currVal) {
                debouncedValue = SETBIT(debouncedValue, i);
            } else {
                debouncedValue = CLRBIT(debouncedValue, i);
            }
        }
    }
    lastValue = currentGPIO;
    return debouncedValue;
}

void reportGPIO(uint8_t myId, uint16_t gpio) {
    Serial.printf("(GPIO)%d-0-%d\r\n", myId, (gpio & 0x00FF) >> 0);
    Serial.printf("(GPIO)%d-1-%d\r\n", myId, (gpio & 0xFF00) >> 8);
}