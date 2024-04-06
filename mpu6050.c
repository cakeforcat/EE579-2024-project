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

    // CONFIG REG: BIT2-0 is DLPF_CFG - Set low pass filter B= 20 Hz w/ 8.3 ms delay (filters gyro noise)
    //             BIT5-3 is EXT_SYNC_SET - SYNC sampling rate to gyro clock (recommended)
    TransmitByte(MPU_CONFIG, 0x0C);

    // CONFIG_GYRO REG: BIT4-3 is FS_SEL -  set to 1000 deg/s
    // 1000 deg/s -> 32.8 LSB per deg/s i.e. for every deg/s sensor output changes by 32.8 LSB
    TransmitByte(MPU_CONFIG_GYRO, 0x11);

}

void initMPU(){

    WakeUpMPU();
    ConfigMPU();
}

float CalibrateGyro(int n_samples){

    int i;
    volatile char gyro_raw_high, gyro_raw_low;
    int16_t gyro_raw;
    float gyro_raw_error = 0.0;


    for (i=0; i < n_samples; i++){

        gyro_raw_high = ReceiveByte(MPU_GYRO_ZOUT_H);
        gyro_raw_low = ReceiveByte(MPU_GYRO_ZOUT_L);

        gyro_raw = (gyro_raw_high<<8) | gyro_raw_low;

        gyro_raw_error += gyro_raw / GYRO_SF;

        if (i == n_samples-1){

            gyro_raw_error = gyro_raw_error / n_samples;
            return gyro_raw_error;
        }

    }

}

volatile float GetZReading(volatile float error){

    volatile char gyro_z_msb, gyro_z_lsb;
    int16_t gyro_raw_z = 0.0;
    volatile float gyro_z;

    gyro_z_msb = ReceiveByte(MPU_GYRO_ZOUT_H);
    gyro_z_lsb = ReceiveByte(MPU_GYRO_ZOUT_L);

    gyro_raw_z = (gyro_z_msb<<8) | gyro_z_lsb;

    gyro_z = (gyro_raw_z/GYRO_SF) - error;

    return gyro_z;

}




