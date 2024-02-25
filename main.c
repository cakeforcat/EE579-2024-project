    /**
 * main.c
 * Created on  : 07 Feb 2024
 * Author      : Chris
 */


#include <msp430.h>

#include "Timers.h"
#include "GPIO.h"


// Define bits for motor inputs - skip p1.3 (connects to switch so leave free for now)
#define IN1 (BIT1)                                          // Motor 1 - Input 1
#define IN2 (BIT2)                                          // Motor 1 - Input 2
#define IN3 (BIT4)                                          // Motor 2 - Input 3
#define IN4 (BIT5)                                          // Motor 2 - Input 4

// Macro for checking if it is time (taken from James' example on MyPlace)
#define IsTime(X) ((current_time.sec == X.sec) && (current_time.ms == X.ms))

// Structure for holding current and scheduled times (taken from James' example on MyPlace)
struct Time {
    int sec;                                                // Stores no of secs elapsed
    int ms;                                                 // Stores no of ms elapsed
};

// Need a different structure which holds us for echo pulse - not accurate enough with just ms
struct PulseTime {
    int ms;
    int us;
};

// Structure for movement commands
struct Movement {
    struct Time start_time;
    struct Time stop_time;
};

// Structure instances to store current time and scheduled events
struct Time current_time = {0, 0};

// Movement structure instances - initing with these vals just to test
struct Movement move_fwd = {{0,500}, {1, 000}};
struct Movement move_bwd = {{1,500}, {2, 000}};
struct Movement move_left = {{2,500}, {3, 000}};
struct Movement move_right = {{3,500}, {4, 000}};

// Sonic vars
struct PulseTime edge_times[2];
volatile float diff;
volatile int e = 0;
volatile float  distance;


// Scheduler function (James' example on MyPlace)
struct Time Schedule (int duration)
{
    struct Time new_time;

    new_time.sec = current_time.sec +
            (current_time.ms + duration) / 1000;            // Calculate secs- adding current seconds and duration in milliseconds

    new_time.ms = (current_time.ms + duration) % 1000;      // Calculate remaining ms
    new_time.sec %= 60;                                     // Ensure that seconds do not exceed 59

    return new_time;                                        // Return scheduled time
}

// Function to calculate distance of detected object
int CalcDistance(struct PulseTime start_time, struct PulseTime end_time) {
    int diff_ms;
    int diff_us;

    // Calculate difference in ms
    if (end_time.ms >= start_time.ms) {
        diff_ms = end_time.ms - start_time.ms;
    } else {
        diff_ms = (1000 - start_time.ms) + end_time.ms;     // Account for ms overflow
    }

    // Calculate difference in us
    if (end_time.us >= start_time.us) {
        diff_us = end_time.us - start_time.us;
    } else {
        diff_us = (1000 - start_time.us) + end_time.us;     // Account for us overflow
        if (diff_ms > 0) {
            diff_ms--;                                      // Borrow from milliseconds if necessary
        }
    }

    diff_us = (diff_ms * 1000) + diff_us;

    return diff_us / 58;
}



//ISR for when 1 ms has elapsed on Timer1 - Scheduler
#pragma vector = TIMER1_A0_VECTOR
__interrupt void TIMER1_ISR0(void)
{

    // Maintaining current time structure
    if (current_time.ms++ == 0x400){                        // Has a second passed?
        current_time.ms = 0;                                // Reset to 0 ms

        if (current_time.sec++ == 60){                      // Increment s and check if = 60
            current_time.sec = 0;                           // Reset to 0 s
        }
    }


    // Start movement in x or y direction
    if IsTime(move_fwd.start_time){
        P1OUT |= IN1;
    }
    if IsTime(move_bwd.start_time){
        P1OUT |= IN2;
    }
    if IsTime(move_left.start_time){
        P1OUT |= IN3;
    }
    if IsTime(move_right.start_time){
        P1OUT |= IN4;
    }

    // Stop movement in x or y direction
    if IsTime(move_fwd.stop_time){
        P1OUT &= ~IN1;
    }
    if IsTime(move_bwd.stop_time){
        P1OUT &= ~IN2;
    }
    if IsTime(move_left.stop_time){
        P1OUT &= ~IN3;
    }
    if IsTime(move_right.stop_time){
        P1OUT &= ~IN4;
    }

    TA0CCTL0 &= ~CCIFG;                                     // Clear IFG
}


// ISR for when rising and falling edge captured on echo pin - Timer1
#pragma vector = TIMER1_A1_VECTOR
__interrupt void Timer_A(void){

        edge_times[e].ms = current_time.ms;                 // Capture ms of edge time - required as in up mode 1 and counter resets after 1 ms (pulse can be longer than 1ms)
        edge_times[e].us = TA1CCR1;                         // Capture us too - needed to achieve any accuracy

        e += 1;                                             // Increment edge index

        TA1CCTL1 &= ~CCIFG ;                                // Clear IFG

        if (e==2) {                                         // i.e. if falling edge detected (end of pulse)

            distance = CalcDistance(edge_times[0], edge_times[1]);

            e=0;
        }
}


int main(void)
{

	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	__enable_interrupt();

	P1DIR |= IN1 + IN2 + IN3 + IN4;                         // Config P1.1, .2, .4, .5 as output pins -> l239d

	P2SEL = BIT1;

	P1DIR |= BIT6;
    P1SEL |= BIT6;

    TA0CTL = TASSEL_2|MC_1 ;
    TA0CCR0 = 0xFFFF;
    TA0CCR1 = 0x000A;
    TA0CCTL1 = OUTMOD_7;


	configTimerA1();
	configSwitch();

	//LPM3;                                                   // ACLK still on - CPU MCLK SMCLK OFF

	return 0;
}


