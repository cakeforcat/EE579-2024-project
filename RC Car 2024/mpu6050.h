/*
 * mpu6050.h
 *
 *  Created on: 5 Apr 2024
 *      Author: Chris
 */

#include "i2c.h"
#include <stdint.h>

#ifndef MPU6050_H_
#define MPU6050_H_


// MPU6050 Register Addresses
// https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Register-Map1.pdf
#define MPU_SLAVE_ADDRESS    0x68
#define MPU_WHO_AM_I         0x75
#define MPU_PWR_MGMT_1       0x6B
#define MPU_PATH_RESET       0x68
#define MPU_CONFIG           0x1A
#define MPU_CONFIG_GYRO      0x1B
#define MPU_GYRO_ZOUT_H      0x47
#define MPU_GYRO_ZOUT_L      0x48
#define MPU_SMPLRT_DIV       0x19

#define GYRO_SF 32.8

extern void WakeUpMPU(void);
extern void ConfigMPU(void);
extern void initMPU(void);
extern float CalibrateGyro(int n_samples);
extern volatile float GetZReading(volatile float error);


#endif /* MPU6050_H_ */

