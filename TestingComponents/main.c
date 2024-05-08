#include <msp430.h>
#include <string.h>
#include <stdlib.h>
char string1[20];
unsigned int i;
int i2=0;

#define TRIG BIT7 // Will be 3.7
#define ECHO BIT5 //Should be 2.5
#include "msp430.h"
#include "I2C.h"
char received;
char buffer[20];
char ToSend[20];

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

  P1SEL = BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
  P1SEL2 = BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD

  UCA0CTL1 |= UCSSEL_1;                     // Using the ACLK
  UCA0BR0 = 0x03;                           // Dividing by 3 gives a baud rate of approximately 9600
  UCA0BR1 = 0x00;                           //
  UCA0MCTL = UCBRS1 + UCBRS0;               // Modulation UCBRSx = 3
  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
  IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt

  BCSCTL1= CALBC1_1MHZ;
  DCOCTL = CALDCO_1MHZ;

  P3DIR |= TRIG; // Set P3.7 as an output
  P2DIR &= ~ECHO; // Set P2.5 as input
  P3OUT &= ~TRIG; //Making sure to start as low

   P3DIR |= BIT0;                             // P3.0 output for servo
   P3SEL |= BIT0;                             // Selection for timer setting
   TA0CCR0 = 20000;                            // CCR0 value (PWM)
   TA0CCTL2 = OUTMOD_7;                        // CCR1 selection reset-set
   TA0CTL = TASSEL_2 | MC_1;                   // SMCLK submain clock, upmode

   initI2C(); //Initialising the I2c so that an address can be read


    __bis_SR_register(GIE);
}


#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)
{
  UCA0TXBUF = string1[i++];                 // Iterating through each character that is stored into string1

  if (i == sizeof string1 - 1)              // Checking if the transmission is over
    IE2 &= ~UCA0TXIE;                       // Disable USCI_A0 TX interrupt
}


#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{

    if (UCA0RXBUF == '1')                      //Case 1, checking for if a one has been received
      {
          i = 0;
          IE2 |= UCA0TXIE;                       // Enabling the interruopt
          send_string("Servo Test\r\n");          // Transmit servo test message

          TA0CCR2 = 500;                          //Angle 1
          __delay_cycles(1500000);
          TA0CCR2 = 1200;                         //Angle 2
          __delay_cycles(1500000);
          TA0CCR2 = 2000;                        //Angle 3
          __delay_cycles(1500000);
          send_string("Finished\r\n");
          send_string("\r\n");

      }

      if (UCA0RXBUF == '2')                      //Checking for when there has been a 2 received
      {
          send_string("IR Test\r\n");
          i2 = 0;
          IE2 |= UCA0TXIE;                       // Enabling the interrupt

          while (i2 < 4)
          {
              i2++;
              if (P3IN & BIT4) //Checking if the pin for the output of the IR is low
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

      if (UCA0RXBUF == '3')
         {
                  i = 0;
                  IE2 |= UCA0TXIE;

                  received = ReceiveByte(0x75);
                  if (received=='h'){ //Since it is a char, the correct WHO_AM_I address is hex number 0x68 which corresponds to h in char
                      send_string("The address read is 0x68\r\n");
                  }
                  else{
                      send_string("Wrong address, try reinitialising\r\n");

                  }
         }


      if (UCA0RXBUF == '4')
         {
                  i = 0;
                  i2=0;
                  IE2 |= UCA0TXIE;
                  long int time = 0;
                  float distance = 0;
                  send_string("Doing the Ultrasonic test\r\n");

                  while(i2 < 4){

                        i2++;
                      // Send a 10us high pulse to trigger the sensor
                        P3OUT |= TRIG;
                        __delay_cycles(11); // 10us delay with 1MHz clock
                        P3OUT &= ~TRIG;
                        __delay_cycles(100);
                        // Initialize time counter
                        time = 0;


                        // Start measuring the length of the high pulse
                        while (!(P2IN & ECHO)) { //The pin is high
                            time++;
                            if(time>=60000){ //To prevent a time out
                                send_string("No reading\r\n");
                                break;
                            }
                        }

                        // Calculate distance in centimeters (speed of sound is 343 m/s, 58 microseconds per cm round trip)
                        distance = ((time * 343) / 2) / 60;
                        //send_int(distance);
                        send_string("\r\n");
                        // Add a short delay before the next cycle

                        }
         }



}

