/*
 * mpu.h
 *
 *  Created on: May 31, 2025
 *      Author: angio
 */

#ifndef SRC_MPU_H_
#define SRC_MPU_H_


typedef struct {
    int pos;
    float vel;
} PosVel;

void mpu_init();
PosVel update_player(int posizione_iniziale, int velocita_iniziale);
float read_accel_z(void);





#endif /* SRC_MPU_H_ */
