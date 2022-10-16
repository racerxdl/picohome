#include "ctrl.h"

#include <Arduino.h>

#include "candata.h"
#include "hardware/watchdog.h"
#include "pb.h"
#include "pb_common.h"
#include "pb_decode.h"
#include "pico/stdlib.h"
#include "shift.pb.h"
#include "config.h"

#define MAX_BYTES 128
#define TIMEOUT 64
#define HC_LED_TIMEOUT 50
#define ERR_LED_TIMEOUT 50

CmdMsg message = CmdMsg_init_zero;
uint16_t numBytes = 0;
uint8_t buffer[MAX_BYTES];
long lastHC = 0;
long lastERR = 0;
long lastPayloadByte = 0;
long timeSinceLastByte = millis();
int lastCount = 0;
can2040_msg ctrlMsg{};

void clearBuff() {
    for (int i = 0; i < MAX_BYTES; i++) {
        buffer[i] = 0;
    }
}

void setError() {
    digitalWrite(LED_ERR, HIGH);
    lastERR = millis();
}

void healthCheckAction() {
    digitalWrite(LED_HC, HIGH);
    lastHC = millis();
    Serial.println("(  HC) Health Check OK");
}

void processPayload() {
    pb_istream_t stream = pb_istream_from_buffer(buffer, numBytes);
    if (!pb_decode(&stream, CmdMsg_fields, &message)) {
        Serial.print("( SER) Error parsing message");
        Serial.println(stream.errmsg);
        setError();
        return;
    }

    switch (message.cmd) {
        case CmdMsg_Command_HealthCheck:
            healthCheckAction();
            ctrlMsg.dlc = 0;
            ctrlMsg.id = CAN2040_ID_EFF | CAN_MANDATORY | (static_cast<uint16_t>(myId) << 8) | CAN_HC_ID;
            sendMessage(&ctrlMsg);
            break;
        case CmdMsg_Command_Reset:
            Serial.println("( ALL) Reset OK");
            // Broken
            // ctrlMsg.dlc = 0;
            // ctrlMsg.id = CAN2040_ID_EFF | CAN_MANDATORY | (static_cast<uint16_t>(myId) << 8) | CAN_RESET_ID;
            // sendMessage(&ctrlMsg);
            // delay(500);
            // watchdog_reboot(0, 0, 0);
            break;
    }
}

void handleUART() {
    int n = Serial.available();
    if (n >= 2) {
        buffer[0] = Serial.read();
        buffer[1] = Serial.read();
        numBytes = *((uint16_t*)buffer);

        if (numBytes >= MAX_BYTES) {
            Serial.print("( ERR) Wanted to receive ");
            Serial.print(numBytes);
            Serial.print("bytes. But max is MAX_BYTES");
            setError();
            return;
        }
        lastPayloadByte = millis();
        while (Serial.available() < numBytes) {
            if (millis() - lastPayloadByte > 1000) {
                Serial.println("( ERR) TIMEOUT");
                setError();
                return;
            }
        }
        clearBuff();
        for (int i = 0; i < numBytes; i++) {
            buffer[i] = Serial.read();
        }

        processPayload();
    }
    if (millis() - lastHC > HC_LED_TIMEOUT) {
        digitalWrite(LED_HC, LOW);
    }

    if (millis() - lastERR > ERR_LED_TIMEOUT) {
        digitalWrite(LED_ERR, LOW);
    }
}
