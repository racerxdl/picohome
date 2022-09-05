#pragma once

#include "can2040.h"

#define CAN_MANDATORY 0x10000
#define CAN_ID_MASK 0x000FF
#define CAN_MYID_MASK 0x0FF00
#define CAN_MSG_ID 0x01
#define CAN_GPIO_ID 0x10
#define CAN_HC_ID 0x11
#define CAN_RESET_ID 0xFF

// TODO: Change me
#define MYID 0

void initCANData();
bool getMessageToSend(can2040_msg *data);
void sendMessage(can2040_msg *data);