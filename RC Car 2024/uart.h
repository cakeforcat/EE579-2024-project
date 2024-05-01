/*
 * uart.h
 *
 *  Created on: 30 Apr 2024
 *      Author: Chris
 */

#include <msp430.h>

#ifndef UART_H_
#define UART_H_


extern void initUART();
extern void send_char(char c);
extern void send_string(const char *str);
extern void send_int(int num);
extern void send_float(float num);
extern int int_to_string(int num, char *buffer);


#endif /* UART_H_ */
