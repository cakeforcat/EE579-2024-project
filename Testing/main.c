#include <msp430.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


char string1[20];
unsigned int i;
int i2;
#define TRIG BIT6 // P1.0
#define ECHO BIT1 // P1.1
int x = 1234;
char buffer[20];
volatile unsigned int time;
volatile unsigned int distance;
float Parsevals;

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

int main(void)
{
    WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
    P3DIR &= ~BIT4;                            // All P1.x outputs
    P1SEL = BIT1 + BIT2;                       // P1.1 = RXD, P1.2 = TXD
    P1SEL2 = BIT1 + BIT2;                      // P1.1 = RXD, P1.2 = TXD
    P1DIR |= TRIG; // Set P1.0 as output (TRIG pin)
    P2DIR &= ~ECHO; // Set P1.1 as input (ECHO pin)
    P1OUT &= ~TRIG; // Ensure TRIG is low initially

    UCA0CTL1 |= UCSSEL_1;                      // CLK = ACLK
    UCA0BR0 = 0x03;                            // 32kHz/9600 = 3.41
    UCA0BR1 = 0x00;
    UCA0MCTL = UCBRS1 + UCBRS0;                // Modulation UCBRSx = 3
    UCA0CTL1 &= ~UCSWRST;                      // **Initialize USCI state machine**
    IE2 |= UCA0RXIE;                           // Enable USCI_A0 RX interrupt

    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    // PWM period
    P3DIR |= BIT0;                             // Set P1.4 as output pin
    P3SEL |= BIT0;                             // Select P1.4 for timer setting
    TA0CCR0 = 20000;                            // PWM period
    TA0CCTL2 = OUTMOD_7;                        // CCR1 selection reset-set
    TA0CTL = TASSEL_2 | MC_1;                   // SMCLK submain clock, upmode
    __bis_SR_register(GIE);        // Enter LPM3 w/ int until Byte RXed


    /*while(1){
        Parsevals=Parsevals+1;
        if(Parsevals==2000){
            Parsevals=0;

        }
    }*/

}

#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)
{
    UCA0TXBUF = string1[i++];                  // TX next character TXBUF used for transmitting and need to do each character at a time

    if (i == sizeof string1 - 1)               // TX over?
        IE2 &= ~UCA0TXIE;                      // Disable USCI_A0 TX interrupt
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{


    if (UCA0RXBUF == '1')                      // Check for specific command
    {
        i = 0;
        IE2 |= UCA0TXIE;                       // Enable USCI_A0 TX interrupt
        send_string("Servo Test\r\n");          // Transmit servo test message

        TA0CCR2 = 500;                          // CCR1 PWM Duty Cycle  !min 350 max 2600 angle 190,
        // 350 2350-180 degrees
        __delay_cycles(1500000);
        TA0CCR2 = 1200;
        __delay_cycles(1500000);
        TA0CCR2 = 2000;
        __delay_cycles(1500000);
        send_string("Finished\r\n");
        send_string("\r\n");

    }

    if (UCA0RXBUF == '2')                      // Check for another specific command
    {
        send_string("IR Test\r\n");
        i2 = 0;
        IE2 |= UCA0TXIE;                       // Enable USCI_A0 TX interrupt

        while (i2 < 4)
        {
            i2++;
            if (P3IN & BIT4)
            {
                i = 0;
                send_string("0\r\n");
                __delay_cycles(187500);
            }
            else
            {
                i = 0;
                send_string("1\r\n");
                __delay_cycles(187500);
            }
            __delay_cycles(500000);
        }
        send_string("\r\n");
    }

}
