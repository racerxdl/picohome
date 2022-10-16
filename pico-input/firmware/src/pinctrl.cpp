#include "pinctrl.h"

#include <Arduino.h>

#include "config.h"

// Millis
#define DEBOUNCE_TIME 50

#define BIT(v, n) (v & (1 << n))
#define SETBIT(v, n) (v | (1 << n))
#define CLRBIT(v, n) (v & ~(1 << n))

uint32_t lastDebounceTime[16];
uint16_t lastValue = 0;
uint16_t debouncedValue = 0;

void initGPIO() {
    pinMode(LED_HC, OUTPUT);   // LED
    pinMode(LED_ACT, OUTPUT);  // LED
    pinMode(LED_ERR, OUTPUT);  // LED
    digitalWrite(LED_HC, LOW);
    digitalWrite(LED_ERR, LOW);
    digitalWrite(LED_ACT, HIGH);
    gpio_set_dir_in_masked(DATA_MASK);
    for (int i = 0; i < 16; i++) {
        lastDebounceTime[i] = 0;
        pinMode(i, INPUT_PULLUP);
    }

    pinMode(ADDR_BIT_0, INPUT_PULLUP);
    pinMode(ADDR_BIT_1, INPUT_PULLUP);
    pinMode(ADDR_BIT_2, INPUT_PULLUP);
    pinMode(ADDR_BIT_3, INPUT_PULLUP);
    pinMode(ADDR_BIT_4, INPUT_PULLUP);

    lastValue = 0xFFFF;
    debouncedValue = 0xFFFF;
}

uint8_t getAddr() {
    return (digitalRead(ADDR_BIT_0) << 0) |
           (digitalRead(ADDR_BIT_1) << 1) |
           (digitalRead(ADDR_BIT_2) << 2) |
           (digitalRead(ADDR_BIT_3) << 3) |
           (digitalRead(ADDR_BIT_4) << 4);
}

uint16_t readGPIO() {
    uint16_t currentGPIO = (gpio_get_all() & DATA_MASK) >> DATA_SHIFT;
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
    return ~debouncedValue;
}

void reportGPIO(uint8_t myId, uint16_t gpio) {
    Serial.printf("(GPIO)%d-0-%d\r\n", myId, (gpio & 0x00FF) >> 0);
    Serial.printf("(GPIO)%d-1-%d\r\n", myId, (gpio & 0xFF00) >> 8);
}