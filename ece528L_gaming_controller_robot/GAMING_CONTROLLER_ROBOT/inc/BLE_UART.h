/**
 * @file BLE_UART.h
 *
 * @brief Header file for the BLE_UART driver.
 *
 * This file contains the function definitions for the BLE_UART driver.
 *
 * It interfaces with the Adafruit Bluefruit LE UART Friend Bluetooth Low Energy (BLE) module, which uses the UART communication protocol.
 *  - Product Link: https://www.adafruit.com/product/2479
 *
 * The following connections must be made:
 *  - BLE UART MOD  (Pin 1)     <-->  MSP432 LaunchPad Pin P1.6
 *  - BLE UART CTS  (Pin 2)     <-->  MSP432 LaunchPad GND
 *  - BLE UART TXO  (Pin 3)     <-->  MSP432 LaunchPad Pin P9.6 (PM_UCA3RXD)
 *  - BLE UART RXI  (Pin 4)     <-->  MSP432 LaunchPad Pin P9.7 (PM_UCA3TXD)
 *  - BLE UART VIN  (Pin 5)     <-->  MSP432 LaunchPad VCC (3.3V)
 *  - BLE UART RTS  (Pin 6)     <-->  Not Connected
 *  - BLE UART GND  (Pin 7)     <-->  MSP432 LaunchPad GND
 *  - BLE UART DFU  (Pin 8)     <-->  Not Connected
 *
 * @note For more information regarding the Enhanced Universal Serial Communication Interface (eUSCI),
 * refer to the MSP432Pxx Microcontrollers Technical Reference Manual
 *
 * @author
 *
 */

#ifndef INC_BLE_UART_H_
#define INC_BLE_UART_H_

#include <stdint.h>
#include <string.h>
#include "msp.h"
#include "Clock.h"

/**
 * @brief Specifies the size of the buffer used for the BLE UART module
 */
#define BLE_UART_BUFFER_SIZE 128

/**
 * @brief Carriage return character
 */
#define CR 0x0D
/**
 * @brief Line feed character
 */
#define LF 0x0A

/**
 * @brief Back space character
 */
#define BS 0x08
/**
 * @brief escape character
 */
#define ESC 0x1B
/**
 * @brief space character
 */
#define SP 0x20
/**
 * @brief delete character
 */
#define DEL 0x7F
/**
 * @brief send character
 */
//#define A 0xAA
//#define B 0x32
//#define C 0xFF
//#define D 0x00

#define F 0x01
#define B 0x02
#define L 0x08
#define R 0x04
#define S 0x00


void BLE_UART_Init();

uint8_t BLE_UART_InChar();

void BLE_UART_OutChar(uint8_t data);

int BLE_UART_InString(char *buffer_pointer, uint16_t buffer_size);

void BLE_UART_OutString(char *pt);

uint8_t Check_BLE_UART_Data(char BLE_UART_Data_Buffer[], char *data_string);

void BLE_UART_Reset();

#endif /* INC_BLE_UART_H_ */
