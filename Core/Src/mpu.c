#include "mpu.h"
#include "stm32f3xx_hal.h"
#include "game.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// Costanti
#define DT 0.02f   // intervallo 10ms in secondi
#define GRAVITY 9.81f
#define ACCEL_SENSITIVITY_2G 16384.0f
// MPU6050 indirizzo I2C (7 bit)
#define MPU6050_ADDR        0x68 << 1  // spostato a 8 bit per HAL


// Registri MPU6050
#define MPU6050_PWR_MGMT_1   0x6B
#define MPU6050_REG_ACCEL_ZOUT_H 0x3F
#define MPU6050_REG_ACCEL_ZOUT_L 0x40
#define MPU6050_ACCEL_CONFIG 0x1C
#define MPU6050_REG_GYRO_YOUT_H 0x45
#define MPU6050_REG_GYRO_YOUT_L 0x46
#define GYRO_SENSITIVITY_250DPS 131.0f  // per ±250°/s

extern I2C_HandleTypeDef hi2c1;
// Dichiarala da qualche parte nel file, magari in cima:
volatile float debug_signal = 0;
extern UART_HandleTypeDef huart2;


void mpu_init(){
	uint8_t data;

	// Wake up the MPU6050 (clear sleep bit)
	data = 0;
	HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, MPU6050_PWR_MGMT_1, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);

	// Set accelerometer full-scale range to ±2g (ACCEL_CONFIG = 0x00)
	data = 0x00;
	HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, MPU6050_ACCEL_CONFIG, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);
}

/*
PosVel update_player(int posizione_iniziale, float velocita_iniziale) {
    static float az_filtered = 0.0f;
    const float alpha = 0.3f;
    const float dead_zone = 0.01f;

    // Accelerazione normalizzata (g) e scalata
    float az_g = read_accel_z() - 1.0f;
    float az_scaled = az_g * 2.0f;

    // Filtro passa basso
    az_filtered = alpha * az_scaled + (1.0f - alpha) * az_filtered;

    // Dead zone
    if (fabsf(az_filtered) < dead_zone) {
        az_filtered = 0.0f;
    }

    debug_signal = az_filtered;

    // Aggiornamento velocità
    float vel_fin = velocita_iniziale + az_filtered * 0.1;
    // Limiti velocità
    const float max_vel = 10.0f;
    if (vel_fin > max_vel) vel_fin = max_vel;
    if (vel_fin < -max_vel) vel_fin = -max_vel;

    float pos_fin_f;
    // Aggiornamento posizione
    if(az_filtered > 0.1 || az_filtered < -0.1){
    	pos_fin_f = posizione_iniziale + 80*az_filtered;
    }
    else{
    	pos_fin_f = posizione_iniziale;
    }
    if (pos_fin_f > (float)(LCD_HEIGHT - 1)) pos_fin_f = (float)(LCD_HEIGHT - 1);
    if (pos_fin_f < 0.0f) pos_fin_f = 0.0f;

    PosVel posvel_fin;
    float diff = posizione_iniziale - pos_fin_f;
    //diff = 20*diff;
    pos_fin_f = posizione_iniziale - diff;
    posvel_fin.pos = (int)(pos_fin_f + 0.5f);
    posvel_fin.vel = vel_fin;

    // ✅ Stampa ogni mezzo secondo
    static int call_count = 0;
    call_count++;
    const int print_interval = (int)(0.5f / DT); // esempio: 0.5 / 0.01 = 50
    if (call_count >= print_interval) {
        call_count = 0;

        char msg[128];
        snprintf(msg, sizeof(msg),
                 "az_filtered=%.2f, vel=%.2f,pos_fin_f = %.2f, pos=%d\r\n",
                 az_filtered, vel_fin,pos_fin_f, posvel_fin.pos);
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    }

    return posvel_fin;
}*/

PosVel update_player(int posizione_iniziale, float velocita_iniziale) {
    PosVel stato;
    static float gyro_filtered = 0.0f;
    const float alpha = 0.2f;
    const float dead_zone = 1.0f; // °/s

    float gyro_y = read_gyro_y(); // velocità angolare in °/s

    // Filtro passa basso
    gyro_filtered = alpha * gyro_y + (1.0f - alpha) * gyro_filtered;

    // Dead zone
    if (fabsf(gyro_filtered) < dead_zone)
        gyro_filtered = 0.0f;

    // Debug
    debug_signal = gyro_filtered;


    // Calcola nuova velocità logica
    float vel_fin = velocita_iniziale + gyro_filtered * DT * 0.5f; // coefficiente da regolare


    // Limiti
    const float max_vel = 80.0f;
    if (vel_fin > max_vel) vel_fin = max_vel;
    if (vel_fin < -max_vel) vel_fin = -max_vel;

    // Posizione
    float pos_fin_f = posizione_iniziale + gyro_filtered*0.1;
    if (pos_fin_f > LCD_HEIGHT - 1) pos_fin_f = LCD_HEIGHT - 1;
    if (pos_fin_f < 0.0f) pos_fin_f = 0.0f;

    stato.pos = (int)(pos_fin_f + 0.5f);
    stato.vel = vel_fin;

    // ✅ Stampa ogni mezzo secondo (debug)
    static int call_count = 0;
    call_count++;
    const int print_interval = (int)(0.5f / DT); // esempio: 0.5 / 0.01 = 50

    if (call_count >= print_interval) {
        call_count = 0;

        char msg[128];
        snprintf(msg, sizeof(msg),
                 "gyro_y=%.2f, filtered=%.2f, vel=%.2f, pos_f=%.2f, pos=%d\r\n",
                 gyro_y, gyro_filtered, vel_fin, pos_fin_f, stato.pos);
        //HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    }

    return stato;
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

float read_gyro_y(){
    uint8_t data[2];
    int16_t raw_gyro_y;
    float gyro_y_dps;

    HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, MPU6050_REG_GYRO_YOUT_H, I2C_MEMADD_SIZE_8BIT, data, 2, HAL_MAX_DELAY);
    raw_gyro_y = (int16_t)(data[0] << 8 | data[1]);
    gyro_y_dps = (float)raw_gyro_y / GYRO_SENSITIVITY_250DPS;

    return gyro_y_dps;
}

void power_off_mpu(void) {
    uint8_t data = 0x40; // Set sleep bit (bit 6) in PWR_MGMT_1
    HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, MPU6050_PWR_MGMT_1, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);
}
