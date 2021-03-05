#include <Arduino.h>

#ifndef CONSTANTS_H
#define CONSTANTS_H

// PECMAC125A I2C address is 2A(42)
const static byte PECMAC125A_ADDR = 0x2A;

const static int MSG_BUFFER_SIZE = 50;

const static uint8_t I2C_SDA = 33;
const static uint8_t I2C_SCL = 32;

const static uint8_t RESET_PIN = 16;

const char* const currentMQTTTopic = "sensors/current";

#endif