/*
 * i2c.h
 *
 *  Created on: 4 Apr 2024
 *      Author: Chris
 */

#include <msp430.h>

#ifndef I2C_H_
#define I2C_H_

#define MPU_SLAVE_ADDRESS      0x68

#define SCL                    BIT6
#define SDA                    BIT7

extern void initI2C();
extern int IsI2CBusy();
extern char ReceiveByte(char register_adress);
extern void TransmitByte(char register_address, char data);

#endif /* I2C_H_ */
