/*
 * Timers.c
 *
 *  Created on: 24 Feb 2024
 *      Author: Chris
 */


#include "Timers.h"

// Function to set each of the timer's registers used for schedulers
void configTimerA1()
{

    // Configure control of timer A1
    TA1CTL |= TASSEL_2;                 // Select SMCLK
    TA1CTL |= MC_1;                     // Select up mode - counts to CCR0 (keep in mind for calcing t for pulse)

    // Configure capture/compare 0 - used for scheduler
    TA1CCTL0 = CCIE;                    // Enable interrupts
    TA1CCR0 = 0x3E8;                    // 1k/1M gives clock period of ~ 1 kHz (T=1ms)

    // Configure capture/compare 1 - used for capturing echo (spreading code for readability)
    TA1CCTL2 |= CAP;                    // Put in capture mode
    TA1CCTL2 |= CCIE;                   // Enable interrupts
    TA1CCTL2 |= CCIS_1;                 // Select CCI2B signal - workaround for PCB layout
    TA1CCTL2 |= CM_3;                   // Capture rising & falling edge (border
    TA1CCTL2 |= SCS;                    // Synchronous capture

    TA1CTL &= ~TAIFG;                   // Clear IFG

}

