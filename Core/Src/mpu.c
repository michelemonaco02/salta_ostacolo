#include "mpu.h"
#include "stm32f3xx_hal.h"
#include "game.h"

// Costanti
#define DT 0.01f   // intervallo 10ms in secondi
#define GRAVITY 9.81f
#define ACCEL_SENSITIVITY_2G 16384.0f
// MPU6050 indirizzo I2C (7 bit)
#define MPU6050_ADDR        0x68 << 1  // spostato a 8 bit per HAL

// Registri MPU6050
#define MPU6050_PWR_MGMT_1   0x6B
#define MPU6050_REG_ACCEL_ZOUT_H 0x3F
#define MPU6050_REG_ACCEL_ZOUT_L 0x40
#define MPU6050_ACCEL_CONFIG 0x1C

extern I2C_HandleTypeDef hi2c1;


void mpu_init(){
	uint8_t data;

	// Wake up the MPU6050 (clear sleep bit)
	data = 0;
	HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, MPU6050_PWR_MGMT_1, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);

	// Set accelerometer full-scale range to ±2g (ACCEL_CONFIG = 0x00)
	data = 0x00;
	HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, MPU6050_ACCEL_CONFIG, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);
}

PosVel update_player(int posizione_iniziale,int velocita_iniziale){

	float az = read_accel_z();
	// Converti accelerazione in m/s²
	float az_m_s2 = az * GRAVITY;

	float vel_fin = velocita_iniziale + az_m_s2*DT;
	int pos_fin = (int) posizione_iniziale + velocita_iniziale*DT + 0.5*az_m_s2*DT*DT;
	if (pos_fin > LCD_HEIGHT){
		pos_fin = LCD_HEIGHT;
	}
	else if(pos_fin < 0.0f){
		pos_fin = 0;
	}

	PosVel posvel_fin;
	posvel_fin.pos = pos_fin;
	posvel_fin.vel = vel_fin;

	return posvel_fin;
}

float read_accel_z(){
	uint8_t data[2];
	int16_t raw_accel_z;
	float accel_z_g;

	// Leggi 2 byte accelerazione Z (H e L)
	HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, MPU6050_REG_ACCEL_ZOUT_H, I2C_MEMADD_SIZE_8BIT, data, 2, HAL_MAX_DELAY);

	raw_accel_z = (int16_t)(data[0] << 8 | data[1]);

	// Converti raw in g
	accel_z_g = (float)raw_accel_z / ACCEL_SENSITIVITY_2G;

	return accel_z_g;
}

