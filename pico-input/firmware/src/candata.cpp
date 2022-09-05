#include "candata.h"

#include <Arduino.h>
extern "C" {
#include "can2040.h"
}

queue_t txMsgQueue;

void initCANData() {
    queue_init(&txMsgQueue, sizeof(can2040_msg), 16);
}

bool getMessageToSend(can2040_msg *data) {
    return queue_try_remove(&txMsgQueue, data);
}

void sendMessage(can2040_msg *data) {
    if (!queue_try_add(&txMsgQueue, data)) {
        Serial.println("( ERR) CAN TX FIFO OVF");
    }
}