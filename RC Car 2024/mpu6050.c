/*
 * mpu6050.c
 *
 *  Created on: 5 Apr 2024
 *      Author: Chris
 */

#include "mpu6050.h"

void WakeUpMPU(){
    // PWR_MGMT_1 REG: BIT6 is SLEEP - we reset this to wake up
    TransmitByte(MPU_PWR_MGMT_1, 0x00);
}

void ConfigMPU(){

    TransmitByte(MPU_PWR_MGMT_1, 0x80);
    __delay_cycles(100000);

    TransmitByte(MPU_PATH_RESET, 0x05);
    __delay_cycles(100000);

    // CONFIG REG: BIT2-0 is DLPF_CFG - Set low pass filter B= 20 Hz w/ 8.3 ms delay (filters gyro noise)
    //             BIT5-3 is EXT_SYNC_SET - SYNC sampling rate to gyro clock (recommended)
    TransmitByte(MPU_CONFIG, 0x0C);

    // CONFIG_GYRO REG: BIT4-3 is FS_SEL -  set to 1000 deg/s
    // 1000 deg/s -> 32.8 LSB per deg/s i.e. for every deg/s sensor output changes by 32.8 LSB
    TransmitByte(MPU_CONFIG_GYRO, 0x10);

}

void initMPU(){

    WakeUpMPU();
    ConfigMPU();
}

// Function to calculate the average error during calibration
float CalibrateGyro(int n_samples){

    int i;
    volatile char gyro_raw_high, gyro_raw_low;
    int16_t gyro_raw;
    float gyro_raw_error = 0.0;


    for (i=0; i < n_samples; i++){

        gyro_raw_high = ReceiveByte(MPU_GYRO_ZOUT_H);       // Read high bitt of gyro z reg
        gyro_raw_low = ReceiveByte(MPU_GYRO_ZOUT_L);        // Read low bits

        gyro_raw = (gyro_raw_high<<8) | gyro_raw_low;       // Concatenate to form 16 bit num

        gyro_raw_error += gyro_raw / GYRO_SF;               // Divide by the scale factor

        if (i == n_samples-1){                              // Last sample?

            gyro_raw_error = gyro_raw_error / n_samples;    // Take the average
            return gyro_raw_error;
        }

    }

}

// Function which returns current Z reading
volatile float GetZReading(volatile float error){

    volatile char gyro_z_msb, gyro_z_lsb;
    int16_t gyro_raw_z = 0.0;
    volatile float gyro_z;

    gyro_z_msb = ReceiveByte(MPU_GYRO_ZOUT_H);               // Same as before - read high and low bits
    gyro_z_lsb = ReceiveByte(MPU_GYRO_ZOUT_L);

    gyro_raw_z = (gyro_z_msb<<8) | gyro_z_lsb;               // Concatenate

    gyro_z = (gyro_raw_z/GYRO_SF) - error;                   // Take away average error

    return gyro_z;

}




