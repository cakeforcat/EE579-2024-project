#include <msp430.h>
unsigned int counter = 0;
int main(void)
{
    P1OUT = 0;
    P2OUT = 0;
    WDTCTL = WDTPW + WDTHOLD; // Stop WDT
    //outputs
    P1DIR |= BIT0|BIT6; //Port 1.0,1.6 as output
    P2DIR |= BIT1|BIT3|BIT5; //Set P2.1,P2.3,P2.5 as an output

    //interrupt
    P1DIR &= ~BIT3; // Set P1.3 as an input
    P1OUT |= BIT3;        //Pull-up Resistor
    P1REN |= BIT3;        //Resistor En.




    CCR0 = 0x3000;
    TA0CTL = TASSEL_1 | MC_1;    //AUX CLK, Count Up to CCR0, /8 (16kHz) 32768
    CCTL0 = CCIE; // CCR0 interrupt enabled
    __enable_interrupt();

}

//Timer A0 interrupt service routine

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{

    if (!(P1IN & BIT3)) // Is switched pressed (==0)?
    {

        if (counter<8)
        {
                P2OUT = 0x0A;
        }
        else if (counter==8)
        {
            P2OUT &= ~0x0A;
            P1OUT |= BIT6; //Set Red Lights
        }
        else if (counter >8)
        {
        P1OUT ^= BIT6; //Toggle
        P2OUT ^= BIT1; //Toggle
        }

    counter++;
    }
    else {
        counter =0;
        P2OUT &= ~BIT1;
        P2OUT &= ~BIT3;
        P2OUT &= ~BIT5;
        P1OUT &= ~BIT6;


    }
}
