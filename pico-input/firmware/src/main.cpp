#include <Arduino.h>

extern "C" {
#include <can2040.h>
}
#include "candata.h"
#include "ctrl.h"
#include "hardware/watchdog.h"
#include "pico/stdlib.h"
#include "pinctrl.h"

can2040 cbus;
queue_t rxMsgQueue;
static uint8_t myId = 0;

static void can2040_cb(can2040 *cd, uint32_t notify, can2040_msg *msg) {
    if (notify == CAN2040_NOTIFY_RX) {
        if (msg->id & CAN_MANDATORY) {
            if (!queue_try_add(&rxMsgQueue, msg)) {
                Serial.println("( ERR) CAN RX FIFO OVF");
                setError();
            }
        }
    }
}

static void PIOx_IRQHandler(void) {
    can2040_pio_irq_handler(&cbus);
}

void canbus_setup(void) {
    uint32_t pio_num = 0;
    uint32_t bitrate = 250000;
    uint32_t gpio_rx = 17, gpio_tx = 16;

    // Setup canbus
    can2040_setup(&cbus, pio_num);
    can2040_callback_config(&cbus, can2040_cb);

    // Enable irqs
    irq_set_exclusive_handler(PIO0_IRQ_0_IRQn, PIOx_IRQHandler);
    NVIC_SetPriority(PIO0_IRQ_0_IRQn, 1);
    NVIC_EnableIRQ(PIO0_IRQ_0_IRQn);

    // Start canbus
    can2040_start(&cbus, F_CPU, bitrate, gpio_rx, gpio_tx);
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("( ALL) Setting up queue");
    queue_init(&rxMsgQueue, sizeof(can2040_msg), 16);
    initCANData();
    Serial.println("( ALL) Setting up CAN");
    canbus_setup();
    initGPIO();
    myId = getAddr();
    Serial.printf("( ALL) MyID: %d\n", myId);
    Serial.println("( ALL) Started");
}

can2040_msg txMsg{};

void loop() {  // CORE1
    // This core should only deal with IRQ from USB, CAN and UART
    // It should also check if there is a message to send.
    if (getMessageToSend(&txMsg)) {
        can2040_transmit(&cbus, &txMsg);
    }
}

void setup1() {  // CORE2
                 // Nothing to setup
}

can2040_msg rxMsg{};
uint16_t lastPinStatus = 0;
char msgBuff[9];

#define IO_CHECK_INTERVAL 5

uint32_t lastIOCheck = 0;

void loop1() {
    // This core should process the CAN messages and report status

    // Handle UART protobuf
    handleUART();

    // Process input messages
    if (queue_try_remove(&rxMsgQueue, &rxMsg)) {
        // We got message from CAN
        uint8_t ID = static_cast<uint8_t>(rxMsg.id & CAN_ID_MASK);
        uint8_t SrcMyID = static_cast<uint8_t>((rxMsg.id & CAN_MYID_MASK) >> 8);
        switch (ID) {
            case CAN_MSG_ID:
                memset(msgBuff, 0x00, 9);
                memcpy(msgBuff, rxMsg.data, rxMsg.dlc > 8 ? 8 : rxMsg.dlc);
                Serial.printf("( ALL) %s\n", msgBuff);
                break;
            case CAN_GPIO_ID:
                reportGPIO(SrcMyID, rxMsg.data32[0]);
                break;
            case CAN_RESET_ID:
                if (SrcMyID == 0xFF || SrcMyID == myId) {
                    Serial.println("( ALL) Received RESET request.");
                    watchdog_reboot(0, 0, 0);
                }
                break;
            case CAN_HC_ID:
                healthCheckAction();
                break;
            default:
                Serial.printf("( ERR) Unknown message ID: %08x\r\n", rxMsg.id);
                setError();
        }
    }

    // Process GPIO
    if (millis() - lastIOCheck > IO_CHECK_INTERVAL) {
        lastIOCheck = millis();
        uint16_t pinStatus = readGPIO();
        if (lastPinStatus != pinStatus) {
            lastPinStatus = pinStatus;
            rxMsg.dlc = 4;
            rxMsg.id = CAN2040_ID_EFF | CAN_MANDATORY | (static_cast<uint16_t>(myId) << 8) | CAN_GPIO_ID;
            rxMsg.data32[0] = static_cast<uint32_t>(pinStatus);
            rxMsg.data32[1] = 0;
            sendMessage(&rxMsg);
            reportGPIO(myId, lastPinStatus);
        }
    }
}