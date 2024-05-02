    /**
 * main.c
 * Created on  : 07 Feb 2024
 * Author      : Chris
 */

#include <msp430.h>
#include <stdlib.h>

#include "Timers.h"
#include "I2C.h"
#include "mpu6050.h"
#include "Definitions.h"

// Structure instances to store current time and scheduled events
struct Time current_time = {0, 0};                          // Self explanatory
struct Time change_duty = {0, -1};                           // Time to schedule change in servo duty cycle
struct Time make_decision = {0, -1};                        // Time to make a decision after scanning
struct Time check_colour = {0, -1};

// Movement structs
struct Pulse move_fwd = {{0,-500}, {0, -600}};
struct Pulse move_fwd_pwm = {{0, -1}, {0, -1}};
struct Pulse move_bwd = {{1,-500}, {1, -600}};
struct Pulse move_bwd_pwm = {{0, -1}, {0, -1}};
struct Pulse move_right = {{2,-500}, {2, -600}};
struct Pulse move_left = {{3,-500}, {3, -600}};
struct Time  turn_time = {0, -1};

// Sonic vars
struct Pulse trig_pulse = {{0, -60}, {0, -1}};               // Trig pulse - PWM 70ms w/ 10ms duty cycle
struct Echo edge_times[2];
volatile int e = 0;
volatile float  distance = 1000;
float avg_distances[11];                                     // Array to store avg distance recorded at 7 positions
int position_index = -1;                                     // Index of array above
int closest_position;

// Servo vars
struct Pulse servo_pulse = {{0, 20}, {0, -1}};
volatile int pos = 0;
unsigned int dir = 0;

// GYRO vars
volatile float gyro_error;
volatile float gyro_angle = 0.0;
struct Pulse gyro_pulse = {{0, 0}, {0, 0}};
float time_elapsed;
float desired_angle;
volatile float delta;

// Algorithm state var
int state = 0;
int turn_count = 0;
int closest_val = 0;
int n_turns;



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

    if (state == GO_PLAY_STATE) {

        if (IsTime(move_fwd.stop_time) || distance <= 40){
            move_fwd.stop_time = current_time;
            move_bwd.start_time = current_time;
            move_bwd.stop_time = Schedule(400);
            change_duty = Schedule(700);
            state = IDLE_STATE;
            //state = WIDE_SCAN_STATE;
            //closest_val = 30;
        }
    }
    if IsTime(make_decision){                               // Time to make decision after scanning?

        if (state == WIDE_SCAN_STATE){
            turn_time = Schedule(210);
            desired_angle = gyro_angle + closest_position;
            state = TURN_STATE;

        } else if (state == NARROW_SCAN_STATE){
            if (IsWall(avg_distances)){                     // See definitions.c
                move_fwd_pwm.start_time = current_time;
                move_fwd.stop_time  = Schedule(500);
                check_colour = Schedule(1000);              // Schedule check colour if it isnt a wall
                state = TURN_STATE;
            } else{
                move_fwd_pwm.start_time = current_time;
                move_fwd.stop_time  = Schedule(500);
                check_colour = Schedule(1000);              // Schedule check colour if it isnt a wall
                state = TURN_STATE;
            }
        }
    }

    if (state == TURN_STATE && IsTime(turn_time)){          // If in turn state and its time to turn

            delta = gyro_angle - desired_angle;

            if (abs(delta) <= 5 || distance <= 25 || (abs(delta) <= 12 && distance <= 60)){
                distance = 1000;
                trig_pulse.start_time = Schedule(500);      // Start measuring distance again
                move_fwd_pwm.start_time = Schedule(200);    // And schedule fwd movement as we're entering Honing state
                move_fwd.stop_time = Schedule(1000);        // Set a timeout for how long we want to hone for in case we miss target
                turn_time = Schedule(-1);
                state = HONING_STATE;

            } else{

                move_bwd_pwm.start_time = Schedule(100);    // Swinging fwd and bwd
                move_bwd.stop_time = Schedule(350);

                move_fwd_pwm.start_time = Schedule(400);
                move_fwd.stop_time = Schedule(700);

                if (delta>0){
                    move_left.start_time = Schedule(100);   // (bwd and left)
                    move_left.stop_time = Schedule(350);

                    move_right.start_time = Schedule(400);  // (fwd and right)
                    move_right.stop_time = Schedule(700);

                } else{

                    move_right.start_time = Schedule(100);
                    move_right.stop_time = Schedule(350);

                    move_left.start_time = Schedule(400);
                    move_left.stop_time = Schedule(700);

                }

                trig_pulse.start_time = Schedule(750);
                turn_time = Schedule(900);
            }
    }

    if (state == HONING_STATE){                             // When in honing state - keep monitoring distance
        if (distance <= 25){                                // If very close
            move_fwd.stop_time = current_time;              // stop

            move_bwd.start_time = current_time;
            move_bwd.stop_time = Schedule(200);

            change_duty = Schedule(400);                    // Scan...
            state = NARROW_SCAN_STATE;                      // Go to NARROW SCAN
        }                                                   // If kind of close or timeout
        else if (distance <= 40||IsTime(move_fwd.stop_time)){
            move_fwd.stop_time = current_time;              // Stop
            change_duty = Schedule(400);                    // Scan...
            state = WIDE_SCAN_STATE;                        // Go to WIDE SCAN
        }
    }

    if IsTime(check_colour){                                // Check IR for colour
        if (P3IN & IR){
            move_bwd.start_time = current_time;             // Go bwd to gain some speed
            move_bwd.stop_time  = Schedule(300);
            move_fwd.start_time = Schedule(400);            // Then hit can
            move_fwd.stop_time = Schedule(1200);
        } else{
            move_bwd.start_time = current_time;
            move_bwd.stop_time = Schedule(500);
        }
        state = WIDE_SCAN_STATE;
        change_duty = Schedule(1300);

    }

    if IsTime(trig_pulse.start_time){                       // Is it time to make TRIG pulse high?
        P3OUT |= TRIG;                                      // Make high
        trig_pulse.stop_time = Schedule(10);                // Schedule to pull back down after 10 ms
    }

    if IsTime(trig_pulse.stop_time){                        // Is=s it time to end TRIG pulse?
        P3OUT &= ~TRIG;                                     // Make low

        if (state==HONING_STATE || state == GO_PLAY_STATE){
            trig_pulse.start_time = Schedule(60);           // Schedule the next TRIG pulse
        }
    }

    if IsTime(change_duty){                                 // Is it time to change duty cycle of PWM for SG90 i.e. change angle

        TA0CCR0 = 20000;                                    // 20 ms PWM period

        if (state == WIDE_SCAN_STATE){                      // Different scan positions dependent on state
            pos = position_index*204;
            TA0CCR2 = 300 + pos;
        } else if (state == NARROW_SCAN_STATE){
            pos = position_index*125;
            TA0CCR2 = 550 + pos;
        }

        if (position_index == 0){                           // Need a longer delay to give time to return to pos 0
            trig_pulse.start_time = Schedule(500);
            change_duty = Schedule(700);
        } else{
            trig_pulse.start_time = Schedule(100);
            change_duty = Schedule(250);
        }
    }

    // Start movement in x or y direction
    if IsTime(move_fwd.start_time){
        P2OUT |= FWD;                                       // Move fwd at 100% speed
    }
    if IsTime(move_bwd.start_time){
        P2OUT |= BWD;                                       // Bwd at 100% speed
    }
    if IsTime(move_right.start_time){
        P3OUT |= RIGHT;
    }
    if IsTime(move_left.start_time){
        P3OUT |= LEFT;
    }

    // Stop movement in x or y direction
    if IsTime(move_fwd.stop_time){
        P2OUT &= ~FWD;                                      // Make pin low
        move_fwd_pwm.start_time = Schedule(-1);             // And stop PWMing
        move_fwd_pwm.stop_time = Schedule(-1);
    }
    if IsTime(move_bwd.stop_time){
        P2OUT &= ~BWD;
        move_bwd_pwm.start_time = Schedule(-1);
        move_bwd_pwm.stop_time = Schedule(-1);
    }
    if IsTime(move_right.stop_time){
        P3OUT &= ~RIGHT;
    }
    if IsTime(move_left.stop_time){
        P3OUT &= ~LEFT;
    }

    // Speed controlled movements
    if IsTime(move_fwd_pwm.start_time){                     // Move Fwd 50% speed
        P2OUT|= FWD;
        move_fwd_pwm.stop_time = Schedule(SPEED_PWM_HIGH);  // Adjust speed in Definitions.h if required
    }
    else if IsTime(move_fwd_pwm.stop_time){
        P2OUT &= ~FWD;
        move_fwd_pwm.start_time = Schedule(SPEED_PWM_LOW);
    }

    if IsTime(move_bwd_pwm.start_time){
        P2OUT |= BWD;
        move_bwd_pwm.stop_time = Schedule(SPEED_PWM_HIGH);
    }
    else if IsTime(move_bwd_pwm.stop_time){
        P2OUT &= ~BWD;
        move_bwd_pwm.start_time = Schedule(SPEED_PWM_LOW);
    }

    TA1CCTL0 &= ~CCIFG;                                     // Clear IFG
}


#pragma vector = TIMER1_A1_VECTOR
__interrupt void Timer_A(void) {
    if (TA1IV == TA1IV_TACCR2) {                            // Check if the interrupt is for CCR2
        edge_times[e].ms = current_time.ms;                 // Capture ms of edge time
        edge_times[e].us = TA1CCR2;                         // Capture us too
        e++;                                                // Increment edge index
        TA1CCTL2 &= ~CCIFG;                                 // Clear CCR2 interrupt flag

        if (e == 2) {                                       // Check if two edges are captured

            distance = CalcDistance(edge_times[0], edge_times[1]);
            e = 0;                                          // Reset edge index

            if (position_index != -1){                      // Get a false echo before trig sent - current workaround...
                avg_distances[position_index] = distance;
            }

            if (state == WIDE_SCAN_STATE || state == NARROW_SCAN_STATE){
                position_index++;                           // Only increment position when in a scan state
            }

            if (position_index == 11){
                position_index = 0;

                if (closest_val){
                    closest_position = FindClosest(avg_distances, closest_val);
                    closest_val = 0;
                } else{
                    closest_position = FindMinIndex(avg_distances);
                }
                //TA0CCR2 = 300 + closest_position*180;
                TA0CCR2 = FWD_POS;                              // Reset servo position to face fwd
                make_decision = Schedule(200);
                change_duty = Schedule(-1);
                distance = 1000.0;
            }
        }
    }
}



int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                               // stop watchdog timer
    __enable_interrupt();

    P1OUT = 0;                                              // Reset all OUT ports
    P2OUT = 0;
    P3OUT = 0;

    P2DIR |= BWD + FWD;                                     // Config Motor A Pins
    P3DIR |= LEFT + RIGHT;                                  // Config Motor B Pins

    TA0CCTL2 = OUTMOD_7;                                    // Config TA0 for PWM
    TA0CTL = TASSEL_2 | MC_1;

    P3SEL |= SERVO;                                         // Config SERVO pin to receive PWM from TA0.2
    P3DIR |= SERVO;
    P3DIR |= TRIG;                                          // Config TRIG pin as output
    P2SEL |= ECHO;                                          // Config ECHO pin for CC
    P3DIR &= ~IR;                                           // IR as input

    initUART();                                             // Init UART
    initI2C();                                              // Init I2C for reading mpu6050
    while ( IsI2CBusy() );

    __delay_cycles(1000000);                                // Need to wait before talking to mpu ~35 ms start up + some time to lay still before calibration

    initMPU();                                              // Wake up MPU & config registers

    gyro_error = CalibrateGyro(100);                        // Get average error of gyro z axis

    configTimerA1();                                        // Congig scheduler clock + echo capture reg

    // Get ready to go
    TA0CCR0 = 20000;
    TA0CCR2 = 1340;
    move_fwd_pwm.start_time = Schedule(500);
    trig_pulse.start_time = Schedule(700);
    move_fwd.stop_time = Schedule(10700);
    state+=1;                                               // Move out of INIT and into go play state

    volatile float temp1 = 0;

    while (1) {

        gyro_pulse.start_time = gyro_pulse.stop_time;
        gyro_pulse.stop_time = current_time;

        time_elapsed = CalcElapsedTime(gyro_pulse.start_time, gyro_pulse.stop_time);
        temp1 = GetZReading(gyro_error) * time_elapsed;

        if (abs(temp1)<=5){
            gyro_angle += temp1;
        } else{
            initI2C();
            while ( IsI2CBusy() );
            initMPU();
        }
    }
    return 0;
}


