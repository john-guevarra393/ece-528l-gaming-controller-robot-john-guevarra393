/**
 * @file BLE_UART.c
 *
 * @brief Source code for the BLE_UART driver.
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

#include "../inc/BLE_UART.h"

void BLE_UART_Init()
{
    //Configure pins P9.6 (PM_UCA3RXD) and P9.7 (PM_UCA3TXD) to use primary module function
    P9->SEL0 |= 0xC0;
    P9->SEL1 &= ~0xC0;

    //Configure P1.6 as an output GPIO (Mode Select)
    P1->SEL0 &= ~0x40;
    P1->SEL1 &= ~0x40;
    P1->DIR |= 0x40;

    //Hold the EUSCI_A3 module in the reset state by setting the
    //UCSWRST bit (Bit 0) in the CTLW0 register
    EUSCI_A3->CTLW0 |= 0x01;

    //Clear all of the bits in the Modulation Control Word (MCTLW) register
    EUSCI_A3->MCTLW &= ~0xFF;

    //Disable the parity bit
    EUSCI_A3->CTLW0 &= ~0x8000;

    //Select odd parity for the parity bit
    EUSCI_A3->CTLW0 &= ~0x4000;

    //Set the bit order to Least Significant Bit (LSB) first
    EUSCI_A3->CTLW0 &= ~0x2000;

    //Select 8-bit character length
    EUSCI_A3->CTLW0 &= ~0x1000;

    //Select one stop bit
    EUSCI_A3->CTLW0 &= ~0x0800;

    //Enable UART mode
    EUSCI_A3->CTLW0 &= ~0x0600;

    //Disable synchronous mode
    EUSCI_A3->CTLW0 &= ~0x0100;

    //Configure the EUSCI_A3 module to use SMCLK as the clock source
    EUSCI_A3->CTLW0 |= 0x00C0;

    //Set the baud rate value
    EUSCI_A3->BRW = 1250;

    //Disable the following interrupts
    EUSCI_A3->IE &= ~0x0C;

    //Enable the following interrupts
    EUSCI_A3->IE |= 0x03;

    //Release the EUSCI_A3 module from the reset state
    EUSCI_A3->CTLW0 &= ~0x01;

}

uint8_t BLE_UART_InChar()
{
    //Check the Receive Interrupt Flag (UCRXIFG, Bit 0)
    //in the IFG register and wait if the flag is not set
    //If the UCRXIFG is set, then the Receive Buffer (UCAxRXBUF) has
    //received a complete character
    while((EUSCI_A3->IFG & 0x01) == 0);

    //Return the data from the Receive Buffer (UCAxRXBUF)
    //Reading the UCAxRXBUF will reset the UCRXIFG flag
    return EUSCI_A3->RXBUF;
}

void BLE_UART_OutChar(uint8_t data)
{
    //Check the Transmit Interrupt flag (UCTXIFG, Bit 1)
    //in the IFG register and wait if the flag is not set
    //If the UCTXIFG is set, then the Transmit Buffer (UCAxTXBUF) is empty
    while((EUSCI_A3->IFG & 0x02) == 0);

    //Write the data to the Transmit Buffer (UCAxTXBUF)
    //Writing to the UCAxTXBUF will clear the UCTXIFG flag
    EUSCI_A3->TXBUF = data;
}

int BLE_UART_InString(char *buffer_pointer, uint16_t buffer_size)
{
    int length = 0;
    int string_size = 0;

    //Read the last received data from the UART Receive Buffer
    char character = BLE_UART_InChar();

    char *start_ptr = buffer_pointer;

    //Check if the received character is a carriage return. Otherwise,
    //if it is a valid character, then store it in Barcode_Scanner_Buffer.
    //For each valid character, increment the string_size variable which will
    //indicate how many characters have been detected from the Barcode Scanner module
    while(character != LF)
    {
        //Remove the character from the buffer is the received character is a backspace character
        if(character == BS)
        {
            if(length)
            {
                buffer_pointer--;
                length--;
                BLE_UART_OutChar(BS);
            }
        }

        //Otherwise, if there are more characters to be read, store them in the buffer
        else if(length < buffer_size)
        {
            *buffer_pointer = character;
            buffer_pointer++;
            length++;
            if (length == 4 && start_ptr[0] == '!' && start_ptr[1] == 'B')
            {
                break;
            }
        }
        character = BLE_UART_InChar();
    }

    *buffer_pointer = '\0';

    return string_size;
}



void BLE_UART_OutString(char *pt)
{
    while(*pt)
    {
        BLE_UART_OutChar(*pt);
        pt++;
    }
}

uint8_t Check_BLE_UART_Data(char BLE_UART_Data_Buffer[], char *data_string)
{
    if(strstr(BLE_UART_Data_Buffer, data_string) != NULL)
    {
        return 0x01;
    }
    else
    {
        return 0x00;
    }
}

void BLE_UART_Reset()
{
    //Switch to CMD mode by setting the MOD pin (P1.6) to 1
    P1->OUT |= 0x40;
    Clock_Delay1ms(1000);

    //Send the system reset command by sending the "ATZ" string
    //to the BLE UART module
    BLE_UART_OutString("ATZ\r\n");
    Clock_Delay1ms(3000);

    //Switch back to DATA mode by clearing the MOD pin (P1.6) to 0
    P1->OUT &= ~0x40;
}
