/*
 * GPIO.c
 *
 *  Created on: 24 Feb 2024
 *      Author: Chris
 */

#include "GPIO.h"


void configSwitch(){

    //Control Registers
    P1DIR &= ~BIT3;                      // Set P1.3 as an input pin - this is the default safe mode but include for completeness
    P1SEL &= ~BIT3;                      // Select GPIO, not special func
    P1OUT |= BIT3;                       // Select pull up
    P1REN |= BIT3;                       // Enable 35k pull up/down resistor

    //Interrupt Registers
    P1IES |= BIT3;                       // High to Low interrupt - this occurs when switch is depressed
    P1IE |= BIT3;                        // Interrupt enable
    P1IFG &= ~BIT3;                      // Clear Interrupt

}




