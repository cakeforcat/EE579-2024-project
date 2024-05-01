/*
 * Definitions.h
 *
 *  Created on: 13 Apr 2024
 *      Author: Chris
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <msp430.h>

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_


// PORT 1
#define RXD     (BIT1)                                       //
#define TXD     (BIT2)
#define SCL     (BIT6)                                       // MPU6050 - I2C clock - defining here for ref (used in i2c.h)
#define SDA     (BIT7)                                       // MPU6050 - I2C Data

// PORT 2
#define BWD     (BIT0)                                       // Motor AIN1
#define FWD     (BIT1)                                       // Motor AIN2
#define ECHO    (BIT5)                                       // HCSR04 - Echo tied to CC with TA1_1

// PORT 3
#define SERVO   (BIT0)                                       // SG90 - PWM from TA0_1
#define RIGHT   (BIT2)                                       // Motor 2 BIN1
#define LEFT    (BIT3)                                       // Motor 2 BIN2
#define IR      (BIT4)                                       // IR Sensor
#define TRIG    (BIT7)                                       // HCSR04 - Trigger pulse GPIO

// PWM DUTY
#define SPEED_PWM_HIGH 5
#define SPEED_PWM_LOW 5

// STATES
#define IDLE_STATE 0
#define GO_PLAY_STATE 1
#define WIDE_SCAN_STATE 2
#define TURN_STATE 3
#define HONING_STATE 4
#define NARROW_SCAN_STATE 4

// Macro for checking if it is time (taken from James' example on MyPlace)
#define IsTime(X) ((current_time.sec == X.sec) && (current_time.ms == X.ms))

// Structures
struct Time {                                               // Time for scheduling tasks
    int sec;
    int ms;
};

struct Echo {                                               // Finer res echo time
    int ms;
    int us;
};

struct Pulse {                                              // Start and stop times for pulses
    struct Time start_time;
    struct Time stop_time;
};

struct Angle{
    float current;
    float desired;
};

// Helper Function Prototypes
extern float CalcDistance(struct Echo start_time, struct Echo end_time);
extern float CalcElapsedTime(struct Time startTime, struct Time stopTime);
extern int FindMinIndex(float array[]);
extern bool IsWall(float array[]);

#endif DEFINITIONS_H_
