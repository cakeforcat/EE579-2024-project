/*
 * i2c.c
 *
 *  Created on: 4 Apr 2024
 *      Author: Chris
 */

#include "i2c.h"

// Function to config I2C regs - 400 kHz operating freq
void initI2C(void) {

    P1SEL |= SCL + SDA;                         // Config P1.6 SCL and 1.7 SDA
    P1SEL2 |= SCL + SDA;
    UCB0CTL1 |= UCSWRST;                        // Enable SW reset - known state
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;       // Make MCU master & put in synchronous mode
    UCB0CTL1 = UCSSEL_2 + UCSWRST;              // SMCLK approx 1 MHz
    UCB0BR0 = 2;                                // /3 to get fast rate approx 400 kHz
    UCB0BR1 = 1;
    UCB0I2CSA = MPU_SLAVE_ADDRESS;              // Set slave address of mpu6050
    UCB0CTL1 &= ~UCSWRST;                       // Exit SW reset now that registers are configured
}

// Function to check if I2C is busy - WARNING:
// According to MSP430 Erreta " the UCBBUSY bit might get stuck to 1" - may cause issues
int IsI2CBusy(){
        if(UCB0STAT & UCBBUSY){
            return 1;                           // BUS BUSY
        } else{
            return 0;                           // BUS NOT BUSY
        }
}

// Function to read a single byte of given I2C Slave Address
char ReceiveByte(char register_address){

    volatile char byte;
    while (UCB0CTL1 & UCTXSTP);                 // Wait for our prev stop condition to be sent (just in case..)
    UCB0CTL1 |= UCTR + UCTXSTT;                 // Lets start in transmit MODE
    while((IFG2 & UCB0TXIFG) == 0);             // Wait for MPU66050 Device Address to be sent
    UCB0TXBUF = register_address;               // After we get ACK, lets send register address we want to read
    while((IFG2 & UCB0TXIFG) == 0);             // Wait for ACK...
    UCB0CTL1 &= ~UCTR ;                         // Now we can receive data from the register address
    UCB0CTL1 |= UCTXSTT + UCTXNACK;             // Send START and we respond with NACK for single byte read
    while (UCB0CTL1 & UCTXSTT);                 // Wait for start to complete...
    UCB0CTL1 |= UCTXSTP;                        // Send stop
    while(!(IFG2 & UCB0RXIFG));                 // Wait until we receive
    byte = UCB0RXBUF;                           // Read the byte
    return byte;

}

void TransmitByte(char register_address, char data){

    while (UCB0CTL1 & UCTXSTP);                 // Wait for our prev stop condition to be sent (just in case..)
    UCB0CTL1 |= UCTR + UCTXSTT;                 //
    while((IFG2 & UCB0TXIFG) == 0);             // Wait for MPU66050 Device Address to be sent
    UCB0TXBUF = register_address;               // After we get ACK, lets send register address we want to read
    while((IFG2 & UCB0TXIFG) == 0);             // Wait until our reg address is sent
    UCB0TXBUF = data;                           // Now we can send our data to that address
    while((IFG2 & UCB0TXIFG) == 0);             // Wait...
    UCB0CTL1 |= UCTXSTP;                        // STOP
    IFG2 &= ~UCB0TXIFG;                         // Clear Flag

}


