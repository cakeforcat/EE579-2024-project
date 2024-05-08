/*
 * Definitions.c
 *
 *  Created on: 13 Apr 2024
 *      Author: Chris
 */

#include "Definitions.h"

// Function to calculate distance of detected object
float CalcDistance(struct Echo start_time, struct Echo end_time) {

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

    return (float)(diff_us) / 58;
}

// Function to calculate time elapsed in second for gyro
float CalcElapsedTime(struct Time startTime, struct Time stopTime) {
    unsigned int diff_sec;
    unsigned int diff_ms;

    // Calculate difference in seconds
    if (stopTime.sec >= startTime.sec) {
        diff_sec = stopTime.sec - startTime.sec;
    } else {
        diff_sec = (60 - startTime.sec) + stopTime.sec;    // Account for seconds overflow
    }

    // Calculate difference in milliseconds
    if (stopTime.ms >= startTime.ms) {
        diff_ms = stopTime.ms - startTime.ms;
    } else {
        diff_ms = (1000 - startTime.ms) + stopTime.ms;     // Account for milliseconds overflow
        if (diff_sec > 0) {
            diff_sec--;                                    // Borrow from seconds if necessary
        }
    }

    // Convert to float and return the total elapsed time
    return (float)diff_sec + (float)diff_ms / 1000.0;
}

// Function to find closest position in distance array
int FindMinIndex(float array[]) {

    int size = 11;
    int min_index = 0;                                      // Assume the first element is the smallest
    float min_value = array[0];                               // Initialise min val

    int i = 0;
    for (i = 1; i < size; i++) {
        if (array[i] < min_value) {
            min_value = array[i];                             // Update min val
            min_index = i;
        }
    }

    return min_index; // Return the index of the smallest number
}

// Function to check if detected object is a wall
bool IsWall(float array[]) {

    int size = 11;

    int i;
    int j;

    // Check for consecutive numbers w/ +- 7 cm
    for (i = 0; i <= size - 5; i++) {
        bool consecutive = true;
        for (j = i + 1; j < i + 5; j++) {
            if (abs(array[j] - array[j - 1]) > 7) {
                consecutive = false;
                break;
            }
        }
        if (consecutive) {
            return true;
        }
    }
    return false;
}



