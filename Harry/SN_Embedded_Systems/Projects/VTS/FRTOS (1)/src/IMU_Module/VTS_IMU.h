/*
 * VTS_IMU.h
 *
 * Created: 22-09-2022 11.19.53 AM
 *  Author: HarishN
 */ 


#ifndef VTS_IMU_H_
#define VTS_IMU_H_
#include "ASF\sam\utils\cmsis\samv71\include\component\twihs.h"
#include "ASF/sam/drivers/Twih/twih.h"
#include "asf.h"
#include "stdio_serial.h"
#include "conf_board.h"
#include "led.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "function.h"
/** Slave Wait Time */
#define WAIT_TIME   10

/** TWI Bus Clock 400kHz */
#define TWIHS_CLK     400000

/** Address of MPU6050 chip */
#define MPU6050_ADDR 0x68
#define WHO_AM_I_REG 0x75
#define PWR_MGMT_1_REG 0x6B
#define PWR_MGMT_2_REG 0x6C
#define SMPLRT_DIV_REG 0x19
#define ACCEL_CONFIG_REG 0x1C
#define ACCEL_XOUT_H_REG 0x3B
#define TEMP_OUT_H_REG 0x41
#define GYRO_CONFIG_REG 0x1B
#define GYRO_XOUT_H_REG 0x43

#define XGOFFS_TC        0x00 // Bit 7 PWR_MODE, bits 6:1 XG_OFFS_TC, bit 0 OTP_BNK_VLD
#define YGOFFS_TC        0x01
#define ZGOFFS_TC        0x02
#define X_FINE_GAIN      0x03 // [7:0] fine gain
#define Y_FINE_GAIN      0x04
#define Z_FINE_GAIN      0x05
#define XA_OFFSET_H      0x06 // User-defined trim values for accelerometer
#define XA_OFFSET_L_TC   0x07
#define YA_OFFSET_H      0x08
#define YA_OFFSET_L_TC   0x09
#define ZA_OFFSET_H      0x0A
#define ZA_OFFSET_L_TC   0x0B
#define SELF_TEST_X      0x0D
#define SELF_TEST_Y      0x0E
#define SELF_TEST_Z      0x0F
#define SELF_TEST_A      0x10
#define XG_OFFS_USRH     0x13  // User-defined trim values for gyroscope; supported in MPU-6050?
#define XG_OFFS_USRL     0x14
#define YG_OFFS_USRH     0x15
#define YG_OFFS_USRL     0x16
#define ZG_OFFS_USRH     0x17
#define ZG_OFFS_USRL     0x18
#define SMPLRT_DIV       0x19
#define CONFIG           0x1A
#define GYRO_CONFIG      0x1B
#define ACCEL_CONFIG     0x1C
#define FF_THR           0x1D  // Free-fall
#define FF_DUR           0x1E  // Free-fall
#define MOT_THR          0x1F  // Motion detection threshold bits [7:0]
#define MOT_DUR          0x20  // Duration counter threshold for motion interrupt generation, 1 kHz rate, LSB = 1 ms
#define ZMOT_THR         0x21  // Zero-motion detection threshold bits [7:0]
#define ZRMOT_DUR        0x22  // Duration counter threshold for zero motion interrupt generation, 16 Hz rate, LSB = 64 ms
#define FIFO_EN          0x23
#define I2C_MST_CTRL     0x24
#define I2C_SLV0_ADDR    0x25
#define I2C_SLV0_REG     0x26
#define I2C_SLV0_CTRL    0x27
#define I2C_SLV1_ADDR    0x28
#define I2C_SLV1_REG     0x29
#define I2C_SLV1_CTRL    0x2A
#define I2C_SLV2_ADDR    0x2B
#define I2C_SLV2_REG     0x2C
#define I2C_SLV2_CTRL    0x2D
#define I2C_SLV3_ADDR    0x2E
#define I2C_SLV3_REG     0x2F
#define I2C_SLV3_CTRL    0x30
#define I2C_SLV4_ADDR    0x31
#define I2C_SLV4_REG     0x32
#define I2C_SLV4_DO      0x33
#define I2C_SLV4_CTRL    0x34
#define I2C_SLV4_DI      0x35
#define I2C_MST_STATUS   0x36
#define INT_PIN_CFG      0x37
#define INT_ENABLE       0x38
#define DMP_INT_STATUS   0x39  // Check DMP interrupt
#define INT_STATUS       0x3A
#define ACCEL_XOUT_H     0x3B
#define ACCEL_XOUT_L     0x3C
#define ACCEL_YOUT_H     0x3D
#define ACCEL_YOUT_L     0x3E
#define ACCEL_ZOUT_H     0x3F
#define ACCEL_ZOUT_L     0x40
#define TEMP_OUT_H       0x41
#define TEMP_OUT_L       0x42
#define GYRO_XOUT_H      0x43
#define GYRO_XOUT_L      0x44
#define GYRO_YOUT_H      0x45
#define GYRO_YOUT_L      0x46
#define GYRO_ZOUT_H      0x47
#define GYRO_ZOUT_L      0x48
#define EXT_SENS_DATA_00 0x49
#define EXT_SENS_DATA_01 0x4A
#define EXT_SENS_DATA_02 0x4B
#define EXT_SENS_DATA_03 0x4C
#define EXT_SENS_DATA_04 0x4D
#define EXT_SENS_DATA_05 0x4E
#define EXT_SENS_DATA_06 0x4F
#define EXT_SENS_DATA_07 0x50
#define EXT_SENS_DATA_08 0x51
#define EXT_SENS_DATA_09 0x52
#define EXT_SENS_DATA_10 0x53
#define EXT_SENS_DATA_11 0x54
#define EXT_SENS_DATA_12 0x55
#define EXT_SENS_DATA_13 0x56
#define EXT_SENS_DATA_14 0x57
#define EXT_SENS_DATA_15 0x58
#define EXT_SENS_DATA_16 0x59
#define EXT_SENS_DATA_17 0x5A
#define EXT_SENS_DATA_18 0x5B
#define EXT_SENS_DATA_19 0x5C
#define EXT_SENS_DATA_20 0x5D
#define EXT_SENS_DATA_21 0x5E
#define EXT_SENS_DATA_22 0x5F
#define EXT_SENS_DATA_23 0x60
#define MOT_DETECT_STATUS 0x61
#define I2C_SLV0_DO      0x63
#define I2C_SLV1_DO      0x64
#define I2C_SLV2_DO      0x65
#define I2C_SLV3_DO      0x66
#define I2C_MST_DELAY_CTRL 0x67
#define SIGNAL_PATH_RESET  0x68
#define MOT_DETECT_CTRL   0x69
#define USER_CTRL        0x6A  // Bit 7 enable DMP, bit 3 reset DMP
#define PWR_MGMT_1       0x6B // Device defaults to the SLEEP mode
#define PWR_MGMT_2       0x6C
#define DMP_BANK         0x6D  // Activates a specific bank in the DMP
#define DMP_RW_PNT       0x6E  // Set read/write pointer to a specific start address in specified DMP bank
#define DMP_REG          0x6F  // Register in DMP from which to read or to which to write
#define DMP_REG_1        0x70
#define DMP_REG_2        0x71
#define FIFO_COUNTH      0x72
#define FIFO_COUNTL      0x73
#define FIFO_R_W         0x74
#define WHO_AM_I_MPU6050 0x75 // Should return 0x68

/** Push button pin definition. */
#define IMU_INTERRUPT_PIO          PIOA
#define IMU_INTERRUPT_ID           ID_PIOA
#define IMU_INTERRUPT_PIN_MSK      (1 << 22)
#define IMU_INTERRUPT_ATTR         (PIO_PULLUP | PIO_IT_FALL_EDGE)

#define GPIO_IMU_INTERRUPT       (PIO_PA22_IDX)
#define GPIO_IMU_INTERRUPT_FLAGS (IOPORT_MODE_PULLUP | IOPORT_MODE_DEBOUNCE)
#define GPIO_IMU_INTERRUPT_SENSE (IOPORT_SENSE_FALLING)


// MPU6050 structure
typedef struct
{

	int16_t Accel_X_RAW;
	int16_t Accel_Y_RAW;
	int16_t Accel_Z_RAW;
	double Ax;
	double Ay;
	double Az;

	int16_t Gyro_X_RAW;
	int16_t Gyro_Y_RAW;
	int16_t Gyro_Z_RAW;
	double Gx;
	double Gy;
	double Gz;
	
	float Temperature;
	
} MPU6050_t;

// Set initial input parameters
enum Ascale {
	AFS_2G = 0,
	AFS_4G,
	AFS_8G,
	AFS_16G
};

enum Gscale {
	GFS_250DPS = 0,
	GFS_500DPS,
	GFS_1000DPS,
	GFS_2000DPS
};
// Specify sensor full scale


int Gscale, Ascale;
float aRes, gRes; // scale resolutions per LSB for the sensors
// Pin definitions
int16_t accelCount[3];           // Stores the 16-bit signed accelerometer sensor output
float ax, ay, az;                // Stores the real accel value in g's
int16_t gyroCount[3];            // Stores the 16-bit signed gyro sensor output
float gyrox, gyroy, gyroz;                // Stores the real gyro value in degrees per seconds
float gyroBias[3], accelBias[3]; // Bias corrections for gyro and accelerometer
int16_t tempCount;               // Stores the internal chip temperature sensor output
float temperature;               // Scaled temperature in degrees Celsius
float SelfTest[6];               // Gyro and accelerometer self-test sensor output
 
 float test_f; 
 double Accel_Z_corrector;
uint32_t i2c_write(Twihs *p_twihs, uint8_t chip_addr, uint32_t mem_addr, uint32_t mem_addr_length, void *buffer, uint32_t buffer_length);
uint32_t i2c_read(Twihs *p_twihs, uint8_t chip_addr, uint32_t mem_addr, uint32_t mem_addr_length, void *buffer, uint32_t buffer_length);
void MPU6050_Read_Accel(Twihs *p_twihs, MPU6050_t *DataStruct);
void MPU6050_Read_Gyro(Twihs *p_twihs, MPU6050_t *DataStruct);
void MPU6050_Read_Temp(Twihs *p_twihs, MPU6050_t *DataStruct);
void MPU6050SelfTest(float * destination);
void calibrateMPU6050(float * dest1, float * dest2);
void initMPU6050(void);
void InitialiseI2C(void);
void readAccelData(int16_t * destination);
void readGyroData(int16_t * destination);
int16_t readTempData(void);
void getAres(void);
void getGres(void);
extern void SelfTest_MPU6050(void);
void GetIMUReadings(void);
void mdelay(uint32_t ul_dly_ticks);

#endif /* VTS_IMU_H_ */