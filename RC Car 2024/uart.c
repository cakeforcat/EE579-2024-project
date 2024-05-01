/*
 * uart.c
 *
 *  Created on: 30 Apr 2024
 *      Author: Chris
 */

#include "uart.h"

void initUART(){
    P1SEL = BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
    P1SEL2 = BIT1 + BIT2 ;                    // P1.1 = RXD, P1.2=TXD
    UCA0CTL1 |= UCSSEL_1;                     // CLK = ACLK
    UCA0BR0 = 0x03;                           // 32kHz/9600 = 3.41
    UCA0BR1 = 0x00;                           //
    UCA0MCTL = UCBRS1 + UCBRS0;               // Modulation UCBRSx = 3
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
    IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
}

void send_char(char c) {
    // Wait for the transmit buffer to be ready
    while (!(IFG2 & UCA0TXIFG)); // Check the UART transmit interrupt flag
    UCA0TXBUF = c;               // Load the character into the UART transmit buffer
}

// Function to send a string over UART
void send_string(const char *str) {
    while (*str) {
        send_char(*str++); // Send each character of the string
    }
}

// Function to send an integer over UART
void send_int(int num) {
    char buffer[12]; // Buffer to hold the integer as a string
    sprintf(buffer, "%d\r\n", num); // Convert the integer to a string
    send_string(buffer); // Send the converted string
}

// Function to send a floating-point number over UART
void send_float(float num) {
    char buffer[20]; // Buffer to hold the float as a string
    int wholePart = (int)num; // Extract the whole part of the float
    int decimalPart = (int)((num - wholePart) * 100); // Extract the first two decimal places

    // Convert whole part to string
    int digits = int_to_string(wholePart, buffer);

    // Add decimal point
    buffer[digits++] = '.';

    // Convert decimal part to string
    digits += int_to_string(decimalPart, buffer + digits);

    // Add line terminator
    buffer[digits++] = '\r';
    buffer[digits++] = '\n';
    buffer[digits] = '\0';

    send_string(buffer); // Send the converted string
}

// Helper function to convert an integer to a string
int int_to_string(int num, char *buffer) {
    int i = 0;
    if (num < 0) {
        buffer[i++] = '-';
        num = -num;
    }
    int temp = num;
    do {
        buffer[i++] = '0' + (temp % 10);
        temp /= 10;
    } while (temp != 0);

    int j;
    // Reverse the string
    for (j = 0; j < i / 2; j++) {
        char temp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = temp;
    }

    return i;
}


