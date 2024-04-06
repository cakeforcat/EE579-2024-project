    /**
 * main.c
 * Created on  : 07 Feb 2024
 * Author      : Chris
 */


#include <msp430.h>
#include <stdlib.h>

#include "Timers.h"
#include "GPIO.h"
#include "I2C.h"
#include "mpu6050.h"

// PORT 1
#define IN1     (BIT0)                                          // Motor 1 - Input 1
#define IN2     (BIT1)                                          // Motor 1 - Input 2
#define SERVO   (BIT2)                                          // SG90 - PWM from TA0_1
#define IN3     (BIT3)                                          // Motor 2 - Input 3
#define IN4     (BIT4)                                          // Motor 2 - Input 4
#define SCL     (BIT6)                                          // MPU6050 - I2C clock
#define SDA     (BIT7)                                          // MPU6050 - I2C Data

// PORT 2
#define TRIG    (BIT0)                                          // HCSR04 - Trigger pulse GPIO
#define ECHO    (BIT1)                                          // HCSR04 - Echo tied to CC with TA1_1

// Macro for checking if it is time (taken from James' example on MyPlace)
#define IsTime(X) ((current_time.sec == X.sec) && (current_time.ms == X.ms))

// Structure for holding current and scheduled times (taken from James' example on MyPlace)
struct Time {
    int sec;                                                // Stores no of secs elapsed
    int ms;                                                 // Stores no of ms elapsed
};

// Need a different structure which holds us for echo pulse - not accurate enough with just ms
struct Echo {
    int ms;
    int us;
};


// Structure for movement commands
struct Pulse {
    struct Time start_time;
    struct Time stop_time;
};

// Structure instances to store current time and scheduled events
struct Time current_time = {0, 0};                          // Self explanatory
struct Time change_duty = {0, 20};                          // Time to schedule change in servo duty cycle
struct Time read_gyro = {0, -1};                            // Time to schedule read of gyro angle

struct Pulse move_fwd = {{0,500}, {-1, 000}};
struct Pulse move_bwd = {{-1,500}, {2, 000}};
struct Pulse move_left = {{2,500}, {3, 000}};
struct Pulse move_right = {{3,500}, {4, 000}};

// Sonic vars
struct Pulse trig_pulse = {{0, 60}, {0, -1}};               // Trig pulse - PWM 70ms w/ 10ms duty cycle
struct Echo edge_times[2];
volatile int e = 0;
volatile float  distance;

// Servo vars
struct Pulse servo_pulse = {{0, 20}, {0, -1}};
volatile int pos = 0;
unsigned int dir = 0;

// GYRO vars
volatile float gyro_error;
volatile float gyro_angle = 0.0;
struct Pulse gyro_pulse = {{0, 0}, {0, 0}};
float time_elapsed;

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
int CalcDistance(struct Echo start_time, struct Echo end_time) {

    unsigned int diff_ms;                                   // Signed int does not have enough range
    unsigned int diff_us;                                   // Will never be -ve

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
            diff_ms--;                                      // Borrow from ms if necessary
        }
    }

    diff_us = (diff_ms * 1000) + diff_us;

    return diff_us / 58;
}

// Function to calculate time elapsed in second for gyro
float CalcElapsedTime(struct Time startTime, struct Time stopTime) {
    unsigned int diff_sec;
    unsigned int diff_ms;

    // Calculate difference in seconds
    if (stopTime.sec >= startTime.sec) {
        diff_sec = stopTime.sec - startTime.sec;
    } else {
        diff_sec = (60 - startTime.sec) + stopTime.sec; // Account for seconds overflow
    }

    // Calculate difference in milliseconds
    if (stopTime.ms >= startTime.ms) {
        diff_ms = stopTime.ms - startTime.ms;
    } else {
        diff_ms = (1000 - startTime.ms) + stopTime.ms; // Account for milliseconds overflow
        if (diff_sec > 0) {
            diff_sec--; // Borrow from seconds if necessary
        }
    }

    // Convert to float and return the total elapsed time
    return (float)diff_sec + (float)diff_ms / 1000.0;
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

    if IsTime(trig_pulse.start_time){                       // Is it time to make TRIG pulse high?
        P2OUT |= TRIG;                                      // Make high
        trig_pulse.stop_time = Schedule(10);                // Schedule to pull back down after 10 ms
    }

    if IsTime(trig_pulse.stop_time){                        // Is=s it time to end TRIG pulse?
        P2OUT &= ~TRIG;                                     // Make low
        trig_pulse.start_time = Schedule(60);               // Schedule the next TRIG pulse
    }

    if IsTime(change_duty){                                 // Is it time to change duty cycle of PWM for SG90 i.e. change angle

        TA0CCR0 = 20000;                                    // 20 ms PWM period

        if (dir == 0){                                      // Check what dir we are currently moving
            TA0CCR1 = 500 + pos;                            // 500 is lower limit
        } else{
            TA0CCR1 = 2500 - pos;                           // 2500 upper limit
        }

        if (pos == 2000){                                   // If we reach limit
            pos = 0;                                        // Reset to pos 0
            dir ^= 1;                                       // And go other dir
        }

        pos+=5;                                             // Increment position
        change_duty = Schedule(10);                         // Schedule next change
    }

    if IsTime(read_gyro){

        gyro_pulse.stop_time = gyro_pulse.start_time;
        gyro_pulse.start_time = current_time;

        time_elapsed = CalcElapsedTime(gyro_pulse.start_time, gyro_pulse.stop_time);
        gyro_angle += GetZReading(gyro_error) * time_elapsed;
        read_gyro = Schedule(20);
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

            distance = CalcDistance(edge_times[0],
                                    edge_times[1]);

            e=0;
        }
}


int main(void)
{

    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    __enable_interrupt();

    P1DIR |= IN1 + IN2 + IN3 + IN4;                         // Config P1.1, .2, .4, .5 as output pins -> l239d

    TA0CCTL1 = OUTMOD_7;                                    // Config TA0 for PWM
    TA0CTL = TASSEL_2 | MC_1;

    P1SEL |= SERVO;                                         // Config SERVO pin to receive PWM from TA0.1
    P1DIR |= SERVO;

    P2DIR |= TRIG;                                          // Config TRIG pin as output
    P2SEL |= ECHO;                                          // Config ECHO pin for CC

    //IFG2 = 0x00;
    initI2C();
    while ( IsI2CBusy() );

    initMPU();

    gyro_error = CalibrateGyro(100);                        // Get average error of gyro z axis
    read_gyro = Schedule(1);                                // Read gyro must only occur after gyro has been calibrated;


    configTimerA1();


    //configSwitch();

    //LPM3;                                                  // ACLK still on - CPU MCLK SMCLK OFF

    while (1) {}
    return 0;
}


