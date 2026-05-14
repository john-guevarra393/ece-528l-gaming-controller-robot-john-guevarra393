/**
 * @file main.c
 *
 * @brief MSP432 receiver for ESP32 gamepad commands.
 *
 * ESP32 receives the Bluetooth controller input, then sends one UART character
 * to the MSP432. The MSP432 controls the TI-RSLK motors.
 *
 * UART wiring:
 * ESP32 GPIO17 / TX2  -> MSP432 P9.6 / UCA3RXD
 * ESP32 GPIO16 / RX2  -> MSP432 P9.7 / UCA3TXD
 * ESP32 GND           -> MSP432 GND
 *
 * Commands:
 * F = forward
 * B = backward
 * L = rotate left
 * R = rotate right
 * S = stop
 *
 * @author John Guevarra
 */

#include <stdint.h>
#include "msp.h"

#include "inc/Clock.h"
#include "inc/CortexM.h"
#include "inc/GPIO.h"
#include "inc/EUSCI_A0_UART.h"
#include "inc/Motor.h"
#include "inc/BLE_UART.h"

// 50% duty cycle
#define PWM_NOMINAL 5000

void Process_ESP32_Command(char command);

int main(void)
{
    // Stop watchdog timer
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

    // Disable interrupts during initialization
    DisableInterrupts();

    // Initialize 48 MHz system clock
    Clock_Init48MHz();

    // Initialize RGB LED, optional for debugging
    LED2_Init();

    // Initialize UART0 printf terminal, optional for debugging
    EUSCI_A0_UART_Init_Printf();

    // Initialize UART3 on P9.6 RX and P9.7 TX
    // This is the same UART used in your BLE_UART driver.
    BLE_UART_Init();

    // Initialize motors
    Motor_Init();

    // Start with motors stopped
    Motor_Stop();

    // Enable interrupts after initialization
    EnableInterrupts();

    printf("MSP432 ready for ESP32 UART commands.\n");

    while (1)
    {
        // Check if UART3 received a byte from the ESP32
        if (EUSCI_A3->IFG & 0x01)
        {
            char command = BLE_UART_InChar();

            printf("Received command: %c\n", command);

            Process_ESP32_Command(command);
        }
    }
}

void Process_ESP32_Command(char command)
{
    switch (command)
    {
        case 'F':
            Motor_Forward(PWM_NOMINAL, PWM_NOMINAL);
            LED2_Output(RGB_LED_GREEN);
            break;

        case 'B':
            Motor_Backward(PWM_NOMINAL, PWM_NOMINAL);
            LED2_Output(RGB_LED_BLUE);
            break;

        case 'L':
            Motor_Left(PWM_NOMINAL, PWM_NOMINAL);
            LED2_Output(RGB_LED_RED);
            break;

        case 'R':
            Motor_Right(PWM_NOMINAL, PWM_NOMINAL);
            LED2_Output(RGB_LED_YELLOW);
            break;

        case 'S':
            Motor_Stop();
            LED2_Output(RGB_LED_OFF);
            break;

        default:
            Motor_Stop();
            LED2_Output(RGB_LED_OFF);
            break;
    }
}
