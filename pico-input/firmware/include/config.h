#pragma once

#define CAN_RXD 17
#define CAN_TXD 16

#define LED_ACT 18
#define LED_HC  19
#define LED_ERR 20

#define DATA_MASK 0xFFFF
#define DATA_SHIFT 0

#define ADDR_BIT_0 21
#define ADDR_BIT_1 22
#define ADDR_BIT_2 26
#define ADDR_BIT_3 27
#define ADDR_BIT_4 28


// set during runtime
static uint8_t myId;